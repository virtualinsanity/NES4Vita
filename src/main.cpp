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

#include "../nes_emu/Nes_Emu.h"
#include "abstract_file.h"
#include <vita2d.h>
#include <psp2/audioout.h>

#define SCREEN_W 960
#define SCREEN_H 544
const SceSize sceUserMainThreadStackSize = 8*1024*1024;

static Nes_Emu *emu;

static int scale;

unsigned int *framebuffer;
vita2d_texture *framebufferTex;
int audio_port;

static uint8_t nes_width = 160;
static uint8_t nes_height = 102;

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

	emu->set_sample_rate(44100);
	emu->set_equalizer(Nes_Emu::nes_eq);
	emu->set_palette_range(0);

	vita2d_texture *tex = vita2d_create_empty_texture(Nes_Emu::image_width, Nes_Emu::image_height);
	void *tex_data = vita2d_texture_get_datap(tex);

	static uint32_t video_buffer[Nes_Emu::image_width * Nes_Emu::image_height];
	emu->set_pixels(video_buffer, Nes_Emu::image_width);

	Auto_File_Reader freader(path);
	emu->load_ines(freader);
	int16_t wave_buf[SCE_AUDIO_MAX_LEN]={0};
	int scale = 2;
	int pos_x = SCREEN_W/2 - (Nes_Emu::image_width/2)*scale;
	int pos_y = SCREEN_H/2 - (Nes_Emu::image_height/2)*scale;
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
				const Nes_Emu::rgb_t& rgb = emu->nes_colors[col];
				unsigned r = rgb.red;
				unsigned g = rgb.green;
				unsigned b = rgb.blue;
				out_pixels[w] = 0xFF000000 | (r << 0) | (g << 8) | (b << 16);
			}
		}

		vita2d_start_drawing();
		vita2d_clear_screen();

		vita2d_draw_texture_scale(tex, pos_x, pos_y, scale, scale);
		vita2d_end_drawing();
		vita2d_swap_buffers();
		emu->read_samples(wave_buf, SCE_AUDIO_MAX_LEN);
		sceAudioOutOutput(audio_port, wave_buf);

	}

	return 0;
}

int main()
{
	printf("Starting NES4Vita by SMOKE");

	vita2d_init();
	printf("vita2d initialized");
	audio_port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_BGM, 256, 44100, SCE_AUDIO_OUT_MODE_MONO);
	printf("audio initialized");
	emu = new Nes_Emu();

	const char *path = "ux0:data/pnes/roms/Mega Man 2 (USA).nes";

	printf("Loading emulator.... %s", path);
	run_emu(path);

	delete emu;

	sceAudioOutReleasePort(audio_port);
	vita2d_fini();
	sceKernelExitProcess(0);
	return 0;
}
