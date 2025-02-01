#include <psp2/types.h>
#include <psp2/io/fcntl.h>

const char* blargg_fopen( SceUID* out, const char path [], int mode );

const char * blargg_fsize( SceUID f, long* out );