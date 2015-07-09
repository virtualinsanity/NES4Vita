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
#include <psp2/moduleinfo.h>

#include "../nes_emu/Nes_Emu.h"
#include <vita2d.h>

PSP2_MODULE_INFO(0, 0, "NES4Vita");

const SceSize sceUserMainThreadStackSize = 8*1024*1024;

static Nes_Emu *emu;

static int scale;

unsigned int *framebuffer;
vita2d_texture *framebufferTex;

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
   { PSP2_CTRL_CIRCLE, JOY_A },
   { PSP2_CTRL_CROSS, JOY_B },
   { PSP2_CTRL_SELECT, JOY_SELECT },
   { PSP2_CTRL_START, JOY_START },
   { PSP2_CTRL_UP, JOY_UP },
   { PSP2_CTRL_DOWN, JOY_DOWN },
   { PSP2_CTRL_LEFT, JOY_LEFT },
   { PSP2_CTRL_RIGHT, JOY_RIGHT },
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

int run_emu(char *path)
{
  SceCtrlData pad;
  unsigned int joypad1, joypad2;

  joypad1 = joypad2 = update_input(&pad);

  emu->emulate_frame(joypad1, joypad2);
  const Nes_Emu::frame_t &frame = emu->frame();

  static uint32_t video_buffer[Nes_Emu::image_width * Nes_Emu::image_height];

  const uint8_t *in_pixels = frame.pixels;
  uint32_t *out_pixels = video_buffer;

  for (unsigned h = 0; h < Nes_Emu::image_height;
        h++, in_pixels += frame.pitch, out_pixels += Nes_Emu::image_width)
  {
	  for (unsigned w = 0; w < Nes_Emu::image_width; w++)
      {
         unsigned col = frame.palette[in_pixels[w]];
         const Nes_Emu::rgb_t& rgb = emu->nes_colors[col];
         unsigned r = rgb.red;
         unsigned g = rgb.green;
         unsigned b = rgb.blue;
         out_pixels[w] = (r << 16) | (g << 8) | (b << 0);
      }
  }
  
  vita2d_start_drawing();
  vita2d_clear_screen();
  vita2d_end_drawing();
  vita2d_swap_buffers();
  
  return 0;
}

int main()
{
	printf("Starting NES4Vita by SMOKE");
	
	vita2d_init();
	printf("vita2d initialized");
	
	char *path = (char*)malloc(sizeof(char) * (strlen("cache0:/VitaDefilerClient/Documents/rom.nes")));
	sprintf(path, "cache0:/VitaDefilerClient/Documents/rom.nes");
	
	printf("Loading emulator.... %s",path);
	run_emu(path);

	vita2d_fini();
	return 0;
}
