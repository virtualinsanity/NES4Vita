#include "abstract_file.h"
// Reads from file on disk
class Std_File_Writer : public Data_Writer {
public:

	// Opens file
	const char * open( const char path [] );

	// Closes file if one was open
	void close();

// Implementation
public:
	Std_File_Writer();
	virtual ~Std_File_Writer();

protected:
	const char *write( const void*, long n );

private:
	void* file_;
};