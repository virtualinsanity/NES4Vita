cmake_minimum_required(VERSION 3.2)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(nes4vita)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set (SRC
        externals/quicknes/nes_emu/abstract_file.cpp
        externals/quicknes/nes_emu/apu_state.cpp
        externals/quicknes/nes_emu/Blip_Buffer.cpp
        externals/quicknes/nes_emu/Data_Reader.cpp
        externals/quicknes/nes_emu/Effects_Buffer.cpp
        externals/quicknes/nes_emu/emu2413.cpp
        externals/quicknes/nes_emu/emu2413_state.cpp
        externals/quicknes/nes_emu/Multi_Buffer.cpp
        externals/quicknes/nes_emu/Nes_Apu.cpp
        externals/quicknes/nes_emu/Nes_Buffer.cpp
        externals/quicknes/nes_emu/Nes_Cart.cpp
        externals/quicknes/nes_emu/Nes_Core.cpp
        externals/quicknes/nes_emu/Nes_Cpu.cpp
        externals/quicknes/nes_emu/nes_data.cpp
        externals/quicknes/nes_emu/Nes_Effects_Buffer.cpp
        externals/quicknes/nes_emu/Nes_Emu.cpp
        externals/quicknes/nes_emu/Nes_File.cpp
        externals/quicknes/nes_emu/Nes_Fme7_Apu.cpp
        externals/quicknes/nes_emu/Nes_Mapper.cpp
        externals/quicknes/nes_emu/Nes_Namco_Apu.cpp
        externals/quicknes/nes_emu/nes_ntsc.cpp
        externals/quicknes/nes_emu/Nes_Oscs.cpp
        externals/quicknes/nes_emu/Nes_Ppu.cpp
        externals/quicknes/nes_emu/Nes_Ppu_Impl.cpp
        externals/quicknes/nes_emu/Nes_Ppu_Rendering.cpp
        externals/quicknes/nes_emu/Nes_State.cpp
        externals/quicknes/nes_emu/nes_util.cpp
        externals/quicknes/nes_emu/Nes_Vrc6_Apu.cpp
        externals/quicknes/nes_emu/Nes_Vrc7.cpp
        src/main.cpp
        src/std_file_reader.cpp
        src/std_file_writer.cpp
        src/file_functions.cpp
)
set(VITA_APP_NAME "Nes Emulator")
set(VITA_TITLEID  "VSDK00018")
set(VITA_VERSION  "01.00")

add_executable(${PROJECT_NAME}
  ${SRC}
)
target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
  externals/quicknes/nes_emu
)
target_link_libraries(${PROJECT_NAME}
  vita2d
  SceDisplay_stub
  SceGxm_stub
  SceSysmodule_stub
  SceCtrl_stub
  ScePgf_stub
  ScePvf_stub
  SceCommonDialog_stub
  freetype
  png
  jpeg
  z
  m
  c
  SceAppMgr_stub
  SceAudio_stub
)

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE vita/sce_sys/icon0.png sce_sys/icon0.png
  FILE vita/sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
  FILE vita/sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
  FILE vita/sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
)
