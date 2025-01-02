NES4Vita
=========

The first NES emulator for the PSVita

Based on Libretro QuickNES core (https://github.com/libretro/QuickNES_Core)

### How to compile
Follow the instructions here to install VITASDK on your PC
https://github.com/vitasdk/vdpm
Run CMake to create the compile files
>cmake -S. -Bbuild
Compile the vpk
>cmake --build build -- -j$(nproc)