#include "std_file_reader.h"
#include "blargg_source.h"
#include "blargg_endian.h"
#include <psp2/types.h>
#include <psp2/io/fcntl.h>

// Std_File_Reader

Std_File_Reader::Std_File_Reader()
{
	file_ = NULL;
}

Std_File_Reader::~Std_File_Reader()
{
	close();
}

static const char* blargg_fopen( SceUID* out, const char path [] )
{
	*out = sceIoOpen( path, SCE_O_RDONLY, 0777);
	if ( !*out )
	{
		return "Unable to open the file";
	}

	return 0;
}

static const char * blargg_fsize( SceUID f, long* out )
{
	*out = sceIoLseek(f,0,SCE_SEEK_END);

	if ( *out < 0 )
		return "Seek error";

	if ( sceIoLseek(f,0,SCE_SEEK_SET) < 0 )
		return "Seek error";

	return 0;
}

const char * Std_File_Reader::open( const char path [] )
{
	close();

	SceUID f;
	RETURN_ERR( blargg_fopen( &f, path ) );

	long s;
	const char * err = blargg_fsize( f, &s );
	if ( err )
	{
		sceIoClose( f );
		return err;
	}

	file_ = (void *)f;
	set_size( s );

	return 0;
}

const char * Std_File_Reader::read_v( void* p, int s )
{
	SceUID f = (SceUID) (file_);
	//if ( (size_t) s != fread( p, 1, s, STATIC_CAST(FILE*, file_) ) )
	int ret = sceIoRead(f,p,s);
	if ( (size_t) s != ret )
	{
		// Data_Reader's wrapper should prevent EOF
		//TODO check( !feof( STATIC_CAST(FILE*, file_) ) );

		return "Error read_v";
	}
	return 0;
}

void Std_File_Reader::close()
{
	if ( file_ )
	{
		SceUID f = (SceUID) (file_);
		sceIoClose(f);
		file_ = NULL;
	}
}
