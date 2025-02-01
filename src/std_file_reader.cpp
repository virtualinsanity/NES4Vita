#include "std_file_reader.h"
#include "blargg_source.h"
#include "blargg_endian.h"
#include "file_functions.h"
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



const char * Std_File_Reader::open( const char path [] )
{
	close();

	SceUID f;
	RETURN_ERR( blargg_fopen( &f, path,  SCE_O_RDONLY) );

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
