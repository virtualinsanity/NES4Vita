#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>

#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/display.h>
#include <psp2/gxm.h>
#include <psp2/types.h>
#include <psp2/kernel/processmgr.h>

#include "Nes_Emu.h"
#include "abstract_file.h"
#include "vita_palette.h"
#include "std_file_reader.h"
#include <vita2d.h>
#include <psp2/audioout.h>
#include <time.h>

#define SCREEN_W 960
#define SCREEN_H 544
#define SAMPLE_COUNT 1024
#define G 1000000000L
#define SHOW_FPS 0
#define FULLSCREEN 0
#define FILE_BUFFER_SIZE 10 * 1024 * 1024
const SceSize sceUserMainThreadStackSize = 8*1024*1024;

static Nes_Emu *emu;

static int scale;

unsigned int *framebuffer;
vita2d_texture *framebufferTex;
static uint8_t nes_width = 160;
static uint8_t nes_height = 102;
int16_t wave_buf[SCE_AUDIO_MAX_LEN]={0};
size_t frame_count = 0, sample_count = 0, buffer_block = 0;
static int vita_audio_thread(SceSize args, void *argp);
struct keymap { unsigned psp2; unsigned nes; };

#define JOY_A           1
#define JOY_B           2
#define JOY_SELECT      4
#define JOY_START       8
#define JOY_UP       0x10
#define JOY_DOWN     0x20
#define JOY_LEFT     0x40
#define JOY_RIGHT    0x80

static const keymap bindmap[] = {
   { SCE_CTRL_CROSS, JOY_A },
   { SCE_CTRL_SQUARE, JOY_B },
   { SCE_CTRL_SELECT, JOY_SELECT },
   { SCE_CTRL_START, JOY_START },
   { SCE_CTRL_UP, JOY_UP },
   { SCE_CTRL_DOWN, JOY_DOWN },
   { SCE_CTRL_LEFT, JOY_LEFT },
   { SCE_CTRL_RIGHT, JOY_RIGHT },
};

unsigned update_input(SceCtrlData *pad)
{
   unsigned res = 0;
   unsigned int keys_down = pad->buttons;

   for (unsigned p = 0; p < 2; p++)
      for (unsigned bind = 0; bind < sizeof(bindmap) / sizeof(bindmap[0]); bind++)
         res |= keys_down & bindmap[bind].psp2 ? bindmap[bind].nes : 0;

   return res;
}

int run_emu(const char *path)
{
	SceCtrlData pad;
	unsigned int joypad1, joypad2;
	vita2d_pgf *pgf = vita2d_load_default_pgf();
	emu->set_sample_rate(48000);
	emu->set_equalizer(Nes_Emu::nes_eq);
	emu->set_palette_range(0);

	vita2d_texture *tex = vita2d_create_empty_texture(Nes_Emu::image_width, Nes_Emu::image_height);
	void *tex_data = vita2d_texture_get_datap(tex);

	static uint32_t video_buffer[Nes_Emu::image_width * Nes_Emu::image_height];
	emu->set_pixels(video_buffer, Nes_Emu::image_width);
	
	Std_File_Reader freader;
	freader.open(path);
	emu->load_ines(freader);
	freader.close();
	#if FULLSCREEN
	float scale = (float)SCREEN_H/(float)Nes_Emu::image_height;
	int pos_y = 0;
	#else
	int scale = 2;
	int pos_y = SCREEN_H/2 - (Nes_Emu::image_height/2)*scale;
	#endif
	int pos_x = SCREEN_W/2 - (Nes_Emu::image_width/2)*scale;
	#if SHOW_FPS
	char msg[512];
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	#endif
	while (1) {
		sceCtrlPeekBufferPositive(0, &pad, 1);
		if (pad.buttons & (SCE_CTRL_START & SCE_CTRL_SELECT)) break;
		joypad1 = joypad2 = update_input(&pad);

		emu->emulate_frame(joypad1, joypad2);
		const Nes_Emu::frame_t &frame = emu->frame();
		const uint8_t *in_pixels = frame.pixels;
		uint32_t *out_pixels = (uint32_t *)tex_data;

		for (unsigned h = 0; h < Nes_Emu::image_height;
			h++, in_pixels += frame.pitch, out_pixels += Nes_Emu::image_width) {
			for (unsigned w = 0; w < Nes_Emu::image_width; w++) {
				unsigned col = frame.palette[in_pixels[w]];
				const uint32_t color = vita_palette[col];
				out_pixels[w] = color;
			}
		}
		
		vita2d_start_drawing();
		vita2d_clear_screen();
		vita2d_draw_texture_scale(tex, pos_x, pos_y, scale, scale);
		#if SHOW_FPS
		clock_gettime(CLOCK_MONOTONIC, &end);
		if(frame_count % 10 == 0){
			int64_t diff_sec = end.tv_sec - start.tv_sec;
			int64_t diff_nsec = end.tv_nsec - start.tv_nsec;
			if(diff_nsec < 0){
				diff_sec--;
				diff_nsec += G;
			}
			int64_t frame_time =  diff_sec * G + diff_nsec;
			sprintf(msg, "FPS %.1f", 1000000000.0f/frame_time);
		}
		start.tv_sec = end.tv_sec;
		start.tv_nsec = end.tv_nsec;
		vita2d_pgf_draw_text(pgf, 0, 30, RGBA8(0,255,0,255), 1.0f, msg);
		#endif
		vita2d_end_drawing();
		vita2d_swap_buffers();
		while(buffer_block); //wait until the buffer is unlocked by the thread
		sample_count += emu->read_samples(wave_buf+sample_count, SAMPLE_COUNT);
		frame_count++;
	}

	return 0;
}

int main()
{
	printf("Starting NES4Vita by SMOKE");

	vita2d_init();
	printf("vita2d initialized");
	emu = new Nes_Emu();

	const char *path = "ux0:data/nes4vita/rom.nes";
	SceUID audiothread = 
		sceKernelCreateThread("Audio Thread", &vita_audio_thread, 
					0x10000100, 0x10000, 0, 0, NULL);
    sceKernelStartThread(audiothread, 0, NULL);
	printf("Loading emulator.... %s", path);
	run_emu(path);

	delete emu;
	vita2d_fini();
	sceKernelExitProcess(0);
	return 0;
}

static int vita_audio_thread(SceSize args, void *argp) {
    int audio_port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_MAIN, SAMPLE_COUNT, 48000, SCE_AUDIO_OUT_MODE_MONO);
	size_t last_frame = 0;
    for (;;) {
		if(frame_count == last_frame)
			continue; //frame already played
		last_frame = frame_count;
		if(sample_count >= SAMPLE_COUNT){
			buffer_block = 1;
			sceAudioOutOutput(audio_port, wave_buf);
			//shift the first (sample_count - SAMPLE_COUNT) samples back
			for(int i = SAMPLE_COUNT; i < sample_count; i++){
				wave_buf[i-SAMPLE_COUNT] = wave_buf[i];
			}
			sample_count = sample_count - SAMPLE_COUNT;
			buffer_block = 0;
		}
    }
    sceAudioOutReleasePort(audio_port);
    return sceKernelExitDeleteThread(0);
}