#include "Data_Reader.h"
// Reads from file on disk
class Std_File_Reader : public File_Reader {
public:

	// Opens file
	const char * open( const char path [] );

	// Closes file if one was open
	void close();

// Implementation
public:
	Std_File_Reader();
	virtual ~Std_File_Reader();

protected:
	virtual const char * read_v( void*, int );

private:
	void* file_;
};