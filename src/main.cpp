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
#include <time.h>

#define SCREEN_W 960
#define SCREEN_H 544
#define SAMPLE_COUNT 1024
#define G 1000000000L
const SceSize sceUserMainThreadStackSize = 8*1024*1024;

static Nes_Emu *emu;

static int scale;

unsigned int *framebuffer;
vita2d_texture *framebufferTex;
int audio_port;
const uint32_t vita_palette[Nes_Emu::color_table_size] = {
	0xff666666,0xff882a00,0xffa81214,0xffa4003b,
	0xff7e005c,0xff40006e,0xff00076c,0xff001d57,
	0xff003534,0xff00490c,0xff005200,0xff084f00,
	0xff4e4000,0xff000000,0xff000000,0xff000000,
	0xffaeaeae,0xffda5f15,0xfffe4042,0xffff2776,
	0xffcd1ba1,0xff7c1eb8,0xff2032b5,0xff004f99,
	0xff006e6c,0xff008738,0xff00940d,0xff329000,
	0xff8e7c00,0xff000000,0xff000000,0xff000000,
	0xfffefefe,0xfffeb064,0xfffe9093,0xfffe77c7,
	0xfffe6af3,0xffcd6efe,0xff7082fe,0xff239feb,
	0xff00bfbd,0xff00d989,0xff30e55d,0xff82e145,
	0xffdfce48,0xff4f4f4f,0xff000000,0xff000000,
	0xfffefefe,0xfffee0c1,0xfffed3d4,0xfffec8e9,
	0xfffec3fb,0xffebc5fe,0xffc6cdfe,0xffa6d9f7,
	0xff95e6e5,0xff97f0d0,0xffabf5be,0xffcdf3b4,
	0xfff3ecb5,0xffb8b8b8,0xff000000,0xff000000,
	0xff4f5372,0xff711700,0xff910020,0xff8d0047,
	0xff670068,0xff29007a,0xff000078,0xff000a63,
	0xff002240,0xff003618,0xff003f00,0xff003c00,
	0xff362d00,0xff000000,0xff000000,0xff000000,
	0xff8f94be,0xffbb4525,0xffe42653,0xffe00d86,
	0xffae01b1,0xff5c04c8,0xff0118c6,0xff0035aa,
	0xff00547c,0xff006d49,0xff007a1e,0xff137606,
	0xff6e6209,0xff000000,0xff000000,0xff000000,
	0xffd7defe,0xfffe8e7a,0xfffe6ea8,0xfffe55dc,
	0xfff748fe,0xffa44cfe,0xff4760fe,0xff007dfe,
	0xff009dd2,0xff00b79e,0xff07c372,0xff59bf5a,
	0xffb6ac5d,0xff4f4f4f,0xff000000,0xff000000,
	0xffd7defe,0xffe9bed6,0xfffab1e9,0xfff8a6fe,
	0xffe4a1fe,0xffc2a3fe,0xff9dabfe,0xff7db7fe,
	0xff6cc4fa,0xff6ecee5,0xff82d3d3,0xffa4d2c9,
	0xffcacacb,0xffb8b8b8,0xff000000,0xff000000,
	0xff406a4b,0xff622e00,0xff821600,0xff7e0320,
	0xff580041,0xff1a0052,0xff000b50,0xff00223b,
	0xff003a18,0xff004d00,0xff005600,0xff005300,
	0xff274400,0xff000000,0xff000000,0xff000000,
	0xff7ab488,0xffa66500,0xffd0451d,0xffcb2c50,
	0xff99207b,0xff482492,0xff003790,0xff005474,
	0xff007446,0xff008d13,0xff009900,0xff009500,
	0xff5a8200,0xff000000,0xff000000,0xff000000,
	0xffbcfecf,0xffe9b733,0xfffe9762,0xfffe7e96,
	0xffdc71c1,0xff8975d9,0xff2d89d6,0xff00a6ba,
	0xff00c68c,0xff00e058,0xff00ec2c,0xff3fe814,
	0xff9bd517,0xff4f4f4f,0xff000000,0xff000000,
	0xffbcfecf,0xffcfe790,0xffe0daa3,0xffdecfb8,
	0xffc9cac9,0xffa8ccd3,0xff82d4d2,0xff63e0c6,
	0xff51edb4,0xff53f79f,0xff68fc8d,0xff89fb83,
	0xffaff384,0xffb8b8b8,0xff000000,0xff000000,
	0xff375353,0xff591700,0xff790000,0xff750028,
	0xff4f0049,0xff11005a,0xff000058,0xff000a43,
	0xff002220,0xff003500,0xff003f00,0xff003c00,
	0xff1e2d00,0xff000000,0xff000000,0xff000000,
	0xff6e9493,0xff9a4500,0xffc42628,0xffbf0c5b,
	0xff8d0086,0xff3c049d,0xff00179b,0xff00347f,
	0xff005451,0xff006d1e,0xff007900,0xff007500,
	0xff4e6200,0xff000000,0xff000000,0xff000000,
	0xffaddedd,0xffd98e41,0xfffe6e70,0xffff54a4,
	0xffcc48d0,0xff7a4ce7,0xff1d5fe5,0xff007dc8,
	0xff009d9a,0xff00b666,0xff00c33a,0xff2fbf22,
	0xff8cab25,0xff4f4f4f,0xff000000,0xff000000,
	0xffaddedd,0xffbfbd9e,0xffd0b0b1,0xffcea6c6,
	0xffb9a1d8,0xff98a3e1,0xff72abe0,0xff53b7d5,
	0xff42c3c2,0xff44cead,0xff58d39b,0xff7ad191,
	0xff9fc992,0xffb8b8b8,0xff000000,0xff000000,
	0xff855757,0xffa71a00,0xffc60205,0xffc3002c,
	0xff9d004d,0xff5e005f,0xff19005d,0xff000e47,
	0xff002624,0xff003900,0xff004200,0xff263f00,
	0xff6c3100,0xff000000,0xff000000,0xff000000,
	0xffd89999,0xfffe4a00,0xfffe2b2e,0xfffe1161,
	0xfff7058c,0xffa509a4,0xff4a1ca1,0xff003985,
	0xff005957,0xff007224,0xff0a7e00,0xff5c7a00,
	0xffb76700,0xff000000,0xff000000,0xff000000,
	0xfffee4e5,0xfffe944a,0xfffe7478,0xfffe5bac,
	0xfffe4ed8,0xfffe52ef,0xffa666ed,0xff5983d0,
	0xff2ea3a2,0xff33bd6e,0xff66c942,0xffb8c52a,
	0xfffeb22d,0xff4f4f4f,0xff000000,0xff000000,
	0xfffee4e5,0xfffec4a6,0xfffeb7b9,0xfffeacce,
	0xfffea7e0,0xfffea9e9,0xfffcb1e8,0xffdcbddd,
	0xffcbcaca,0xffcdd4b5,0xffe2d9a3,0xfffed899,
	0xfffed09a,0xffb8b8b8,0xff000000,0xff000000,
	0xff61475a,0xff820b00,0xffa20008,0xff9e002f,
	0xff780050,0xff3a0062,0xff000060,0xff00004a,
	0xff001627,0xff002a00,0xff003300,0xff023000,
	0xff482100,0xff000000,0xff000000,0xff000000,
	0xffa6849e,0xffd23504,0xfffc1632,0xfff70065,
	0xffc50090,0xff7400a8,0xff1907a5,0xff002489,
	0xff00445b,0xff005d28,0xff006900,0xff2a6500,
	0xff865200,0xff000000,0xff000000,0xff000000,
	0xfff6c9ea,0xfffe794f,0xfffe597d,0xfffe3fb1,
	0xfffe33dd,0xffc337f5,0xff664af2,0xff1868d6,
	0xff0088a7,0xff00a173,0xff25ae47,0xff78aa30,
	0xffd59632,0xff4f4f4f,0xff000000,0xff000000,
	0xfff6c9ea,0xfffea8ab,0xfffe9bbe,0xfffe91d3,
	0xfffe8ce5,0xffe18eef,0xffbb96ed,0xff9ca2e2,
	0xff8baecf,0xff8db9ba,0xffa1bea8,0xffc3bc9f,
	0xffe8b4a0,0xffb8b8b8,0xff000000,0xff000000,
	0xff585542,0xff791900,0xff990100,0xff950017,
	0xff6f0038,0xff31004a,0xff000048,0xff000c33,
	0xff002410,0xff003700,0xff004100,0xff003e00,
	0xff3f2f00,0xff000000,0xff000000,0xff000000,
	0xff9a977d,0xffc64800,0xfff02811,0xffeb0f45,
	0xffb90370,0xff680787,0xff0c1a84,0xff003768,
	0xff00573b,0xff007007,0xff007c00,0xff1e7800,
	0xff796500,0xff000000,0xff000000,0xff000000,
	0xffe6e1c0,0xfffe9125,0xfffe7253,0xfffe5887,
	0xfffe4cb3,0xffb350ca,0xff5663c8,0xff0881ab,
	0xff00a07d,0xff00ba49,0xff15c61d,0xff68c205,
	0xffc5af08,0xff4f4f4f,0xff000000,0xff000000,
	0xffe6e1c0,0xfff8c181,0xfffeb494,0xfffeaaa9,
	0xfff2a5bb,0xffd1a6c4,0xffabaec3,0xff8cbab8,
	0xff7bc7a5,0xff7dd190,0xff91d67e,0xffb3d574,
	0xffd8cd76,0xffb8b8b8,0xff000000,0xff000000,
	0xff454545,0xff6e1000,0xff8e0000,0xff8a0021,
	0xff640042,0xff260054,0xff000052,0xff00033c,
	0xff001b19,0xff002e00,0xff003800,0xff003500,
	0xff332600,0xff000000,0xff000000,0xff000000,
	0xff868686,0xffbb4000,0xffe42023,0xffdf0756,
	0xffae0081,0xff5c0099,0xff011296,0xff002f7a,
	0xff004f4c,0xff006819,0xff007400,0xff137000,
	0xff6e5d00,0xff000000,0xff000000,0xff000000,
	0xffcfcfcf,0xfffe883c,0xfffe686b,0xfffe4f9f,
	0xfff842cb,0xffa546e2,0xff485ae0,0xff0077c3,
	0xff009795,0xff00b161,0xff08bd35,0xff5bb91d,
	0xffb7a620,0xff4f4f4f,0xff000000,0xff000000,
	0xffcfcfcf,0xffe5b294,0xfff6a5a6,0xfff49bbc,
	0xffe096cd,0xffbe98d7,0xff989fd6,0xff79abca,
	0xff68b8b7,0xff6ac3a2,0xff7ec891,0xffa0c687,
	0xffc5be88,0xffb8b8b8,0xff000000,0xff000000
};
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
	char msg[512];
	size_t frame_count = 0;
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

	Auto_File_Reader freader(path);
	emu->load_ines(freader);
	int16_t wave_buf[SCE_AUDIO_MAX_LEN]={0};
	int scale = 2;
	int pos_x = SCREEN_W/2 - (Nes_Emu::image_width/2)*scale;
	int pos_y = SCREEN_H/2 - (Nes_Emu::image_height/2)*scale;
	size_t sample_count = 0;
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	while (1) {
		frame_count++;
		sceCtrlPeekBufferPositive(0, &pad, 1);
		if (pad.buttons & (SCE_CTRL_START & SCE_CTRL_SELECT)) break;
		joypad1 = joypad2 = update_input(&pad);

		emu->emulate_frame(joypad1, joypad2);
		const Nes_Emu::frame_t &frame = emu->frame();
		long samples = emu->read_samples(wave_buf+sample_count, SAMPLE_COUNT);
		if((sample_count += samples) >= SAMPLE_COUNT){
			sceAudioOutOutput(audio_port, wave_buf);
			//shift the first SAMPLE_COUNT samples back
			for(int i = SAMPLE_COUNT; i < sample_count; i++){
				wave_buf[i-SAMPLE_COUNT] = wave_buf[i];
			}
			sample_count = sample_count - SAMPLE_COUNT;
		}
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
		clock_gettime(CLOCK_MONOTONIC, &end);
		if(frame_count % 10 == 0){
			int64_t diff_sec = end.tv_sec - start.tv_sec;
			int64_t diff_nsec = end.tv_nsec - start.tv_nsec;
			if(diff_nsec < 0){
				diff_sec--;
				diff_nsec += G;
			}
			int64_t frame_time =  diff_sec * G + diff_nsec;
			sprintf(msg, "FPS %.1f sample_count %d", 1000000000.0f/frame_time, sample_count);
		}
		start.tv_sec = end.tv_sec;
		start.tv_nsec = end.tv_nsec;
		vita2d_pgf_draw_text(pgf, 0, 30, RGBA8(0,255,0,255), 1.0f, msg);
		vita2d_end_drawing();
		vita2d_swap_buffers();
	}

	return 0;
}

int main()
{
	printf("Starting NES4Vita by SMOKE");

	vita2d_init();
	printf("vita2d initialized");
	audio_port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_BGM, SAMPLE_COUNT, 48000, SCE_AUDIO_OUT_MODE_MONO);
	printf("audio initialized");
	emu = new Nes_Emu();

	const char *path = "ux0:data/nes4vita/rom.nes";

	printf("Loading emulator.... %s", path);
	run_emu(path);

	delete emu;

	sceAudioOutReleasePort(audio_port);
	vita2d_fini();
	sceKernelExitProcess(0);
	return 0;
}
