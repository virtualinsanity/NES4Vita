#include "std_file_writer.h"
#include "blargg_source.h"
#include "blargg_endian.h"
#include "file_functions.h"
#include <psp2/types.h>
#include <psp2/io/fcntl.h>

Std_File_Writer::Std_File_Writer(){
    file_ = NULL;
}

Std_File_Writer::~Std_File_Writer()
{
	close();
}

const char * Std_File_Writer::open( const char path [] )
{
	close();

	SceUID f;
	RETURN_ERR( blargg_fopen( &f, path, SCE_O_WRONLY | SCE_O_CREAT ) );

	file_ = (void *)f;

	return 0;
}
const char *Std_File_Writer::write( const void* buff, long n ){
    SceUID f = (SceUID) (file_);
	//if ( (size_t) s != fread( p, 1, s, STATIC_CAST(FILE*, file_) ) )
	int ret = sceIoWrite(f, buff, n);
	if ( (size_t) n != ret )
	{
		// Data_Reader's wrapper should prevent EOF
		//TODO check( !feof( STATIC_CAST(FILE*, file_) ) );

		return "Error write";
	}
	return 0;
}

void Std_File_Writer::close()
{
	if ( file_ )
	{
		SceUID f = (SceUID) (file_);
		sceIoClose(f);
		file_ = NULL;
	}
}