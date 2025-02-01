#include <psp2/types.h>
#include <psp2/io/fcntl.h>

const char* blargg_fopen( SceUID* out, const char path [], int mode )
{
	*out = sceIoOpen( path, mode, 0777);
	if ( !*out )
	{
		return "Unable to open the file";
	}

	return 0;
}

const char * blargg_fsize( SceUID f, long* out )
{
	*out = sceIoLseek(f,0,SCE_SEEK_END);

	if ( *out < 0 )
		return "Seek error";

	if ( sceIoLseek(f,0,SCE_SEEK_SET) < 0 )
		return "Seek error";

	return 0;
}