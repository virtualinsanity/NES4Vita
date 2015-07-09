#
# Copyright (c) 2015 Sergi Granell (xerpi)
# based on Cirne's vita-toolchain test Makefile
#

TARGET		:= NES4Vita
NES_EMU		:= nes_emu
FEX			:= fex
SOURCES		:= src
INCLUDES	:= src

BUILD_APP=$(NES_EMU)/apu_state.o $(NES_EMU)/Blip_Buffer.o $(NES_EMU)/Effects_Buffer.o \
	$(NES_EMU)/Mapper_Fme7.o $(NES_EMU)/Mapper_Mmc5.o $(NES_EMU)/Mapper_Namco106.o \
	$(NES_EMU)/Mapper_Vrc6.o $(NES_EMU)/misc_mappers.o $(NES_EMU)/Multi_Buffer.o $(NES_EMU)/Nes_Apu.o \
	$(NES_EMU)/Nes_Buffer.o $(NES_EMU)/Nes_Cart.o $(NES_EMU)/Nes_Core.o $(NES_EMU)/Nes_Cpu.o \
	$(NES_EMU)/nes_data.o $(NES_EMU)/Nes_Effects_Buffer.o $(NES_EMU)/Nes_Emu.o \
	$(NES_EMU)/Nes_Fme7_Apu.o $(NES_EMU)/Nes_Mapper.o $(NES_EMU)/nes_mappers.o $(NES_EMU)/Nes_Mmc1.o \
	$(NES_EMU)/Nes_Mmc3.o $(NES_EMU)/Nes_Namco_Apu.o $(NES_EMU)/Nes_Oscs.o $(NES_EMU)/Nes_Ppu.o \
	$(NES_EMU)/Nes_Ppu_Impl.o $(NES_EMU)/Nes_Ppu_Rendering.o $(NES_EMU)/nes_util.o $(NES_EMU)/Nes_Vrc6_Apu.o \
	$(FEX)/Data_Reader.o

CFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c))
CXXFILES := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp))
OBJS     := $(CFILES:.c=.o) $(BUILD_APP) $(CXXFILES:.cpp=.o)

LIBS = -lSceDisplay_stub -lSceGxm_stub -lSceCtrl_stub -lc_stub -lm_stub -lvita2d

DEFINES	= -Wall -Wno-multichar -Wno-unused-variable -Wno-sign-compare -Wno-strict-aliasing

PREFIX  = arm-none-eabi
AS		= $(PREFIX)-as
CC      = $(PREFIX)-gcc
CXX		= $(PREFIX)-g++
READELF = $(PREFIX)-readelf
OBJDUMP = $(PREFIX)-objdump
CFLAGS  = -Wall -specs=psp2.specs
CXXFLAGS = $(CFLAGS) $(DEFINES) -O2 -fno-unwind-tables -fno-rtti -fno-exceptions -Wno-deprecated -Wno-comment -Wno-sequence-point -std=c++11
ASFLAGS = $(CFLAGS)

all: $(TARGET).velf

$(TARGET).velf: $(TARGET).elf
	psp2-fixup -q -S $< $@

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).elf $(TARGET).velf $(OBJS)
