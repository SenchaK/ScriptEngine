#include "VMFileSystem.h"

static void filesystemInit( FILE_SYSTEM& thisObj ){
	thisObj.fp = NULL;
	thisObj.handleId = 0;
}

int FileSystemLibrary::findEmptyFileHandle(){
	for( int i = 0 ; i < sizeof(fileSystem)/sizeof(*fileSystem) ; i++ ){
		if( !fileSystem[i].fp ){
			return i;
		}
	}
	return -1;
}

FILE_SYSTEM* FileSystemLibrary::getFileHandle( int handleid ){
	int index = 0xFFFF & handleid;
	if( index < 0 || index >= sizeof(fileSystem)/sizeof(*fileSystem) ) return NULL;
	if( fileSystem[index].handleId == handleid ){
		return &fileSystem[index];
	}
	return NULL;
}



FileSystemLibrary::FileSystemLibrary() : SenchaVM::Assembly::VMSystemCallService(){
	serial = 0;
	filesystemInit( fileSystem[0] );
	filesystemInit( fileSystem[1] );
	filesystemInit( fileSystem[2] );
	filesystemInit( fileSystem[3] );
	filesystemInit( fileSystem[4] );
}



/* virtual */
void FileSystemLibrary::callFunction( string funcName ){
	if( funcName == "File_Open" ){
		const string& mode = popMemory().ValueString();
		const string& fileName = popMemory().ValueString();
		File_Open( fileName , mode );
	}
	else if( funcName == "File_Write" ){
		const string& str = popMemory().ValueString();
		const unsigned int& handleid = (unsigned int)popMemory().Value();
		FILE_SYSTEM* file = getFileHandle( (int)handleid );
		assert( file );
		assert( file->fp );
		File_Write( file->fp , str );
	}
	else if( funcName == "File_Write_u32" ){
		unsigned long data = (unsigned int)popMemory().Value();
		unsigned int handleid = (unsigned int)popMemory().Value();
		FILE_SYSTEM* file = getFileHandle( (int)handleid );
		assert( file );
		assert( file->fp );
		File_Write_u32( file->fp , data );
	}
	else if( funcName == "File_Write_u16" ){
		const unsigned short& data = (unsigned short)popMemory().Value();
		const unsigned int& handleid = (unsigned int)popMemory().Value();
		FILE_SYSTEM* file = getFileHandle( (int)handleid );
		assert( file );
		assert( file->fp );
		File_Write_u16( file->fp , data );
	}
	else if( funcName == "File_Write_u8" ){
		const unsigned char& data = (unsigned char)popMemory().Value();
		const unsigned int& handleid = (unsigned int)popMemory().Value();
		FILE_SYSTEM* file = getFileHandle( (int)handleid );
		assert( file );
		assert( file->fp );
		File_Write_u8( file->fp , data );
	}
	else if( funcName == "File_Write_string" ){
		const string& data = popMemory().ValueString();
		const unsigned int& handleid = (unsigned int)popMemory().Value();
		FILE_SYSTEM* file = getFileHandle( (int)handleid );
		assert( file );
		assert( file->fp );
		File_Write_string( file->fp , data );
	}	
	else if( funcName == "File_GetSize" ){
		const unsigned int& handleid = (unsigned int)popMemory().Value();
		//cout << "HandleID : " << handleid << endl;
		FILE_SYSTEM* file = getFileHandle( (int)handleid );
		assert( file );
		assert( file->fp );
		File_GetSize( file->fp );
	}
	else if( funcName == "File_GetChar" ){
		const unsigned int& handleid = (unsigned int)popMemory().Value();
		FILE_SYSTEM* file = getFileHandle( (int)handleid );
		assert( file );
		assert( file->fp );
		File_GetChar( file->fp );
	}
	else if( funcName == "File_ReadBytes" ){
		const int& count = (int)popMemory().Value();
		const unsigned int& handleid = (unsigned int)popMemory().Value();
		FILE_SYSTEM* file = getFileHandle( (int)handleid );
		assert( file );
		assert( file->fp );
		File_ReadBytes( file->fp , count );
	}
	else if( funcName == "File_Read_u32" ){
		const unsigned int& handleid = (unsigned int)popMemory().Value();
		FILE_SYSTEM* file = getFileHandle( (int)handleid );
		assert( file );
		assert( file->fp );
		File_Read_u32( file->fp );
	}
	else if( funcName == "File_Read_u16" ){
		const unsigned int& handleid = (unsigned int)popMemory().Value();
		FILE_SYSTEM* file = getFileHandle( (int)handleid );
		assert( file );
		assert( file->fp );
		File_Read_u16( file->fp );
	}
	else if( funcName == "File_Read_u8" ){
		const unsigned int& handleid = (unsigned int)popMemory().Value();
		FILE_SYSTEM* file = getFileHandle( (int)handleid );
		assert( file );
		assert( file->fp );
		File_Read_u8( file->fp );
	}
	else if( funcName == "File_Read_string" ){
		const unsigned int& handleid = (unsigned int)popMemory().Value();
		FILE_SYSTEM* file = getFileHandle( (int)handleid );
		assert( file );
		assert( file->fp );
		File_Read_string( file->fp );
	}
	else if( funcName == "File_IsEof" ){
		const unsigned int& handleid = (unsigned int)popMemory().Value();
		FILE_SYSTEM* file = getFileHandle( (int)handleid );
		assert( file );
		assert( file->fp );
		File_IsEof( file->fp );
	}
	else if( funcName == "File_Close" ){
		const unsigned int& handleid = (unsigned int)popMemory().Value();
		File_Close( handleid );
	}
}



void FileSystemLibrary::File_Open( const string& fileName , const string& mode ){
	int index = findEmptyFileHandle();
	assert( index >= 0 );
	serial++;
	fileSystem[index].handleId = ( serial << 16 ) | index;
	fileSystem[index].fp = fopen( fileName.c_str() , mode.c_str() );
	assert( fileSystem[index].fp );
	Return( (double)fileSystem[index].handleId );
}
void FileSystemLibrary::File_Write( FILE* fp , const string& value ){
	fwrite( value.c_str() , 1 , value.length() , fp );
}
void FileSystemLibrary::File_Write_u32( FILE* fp , const unsigned long&  u32value ){
	fwrite( &u32value , 4 , 1 , fp );
}
void FileSystemLibrary::File_Write_u16( FILE* fp , const unsigned short& u16value ){
	fwrite( &u16value , 2 , 1 , fp );
}
void FileSystemLibrary::File_Write_u8( FILE* fp , const unsigned char&  u8value  ){
	fwrite( &u8value , 1 , 1 , fp );
}
void FileSystemLibrary::File_Write_string( FILE* fp , const string&  string_value ){
	fwrite( string_value.c_str() , 1 , string_value.length() + 1 , fp ); 
}
void FileSystemLibrary::File_GetSize( FILE* fp ){
	fpos_t pos = 0;
	fpos_t fsize = 0;
	fgetpos( fp , &fsize );
	fseek  ( fp , 0 , SEEK_SET );
	fseek  ( fp , 0 , SEEK_END ); 
	fgetpos( fp , &fsize );
	fsetpos( fp , &pos );
	Return( (double)fsize );
}
void FileSystemLibrary::File_GetChar( FILE* fp ){
	int ret = fgetc( fp );
	string str = "";
	str += (char)ret;
	Return( (char)ret , str );
}
void FileSystemLibrary::File_ReadBytes( FILE* fp , int count ){
	char* buffer = new char[count];
	memset( buffer , 0 , sizeof( char ) * count );
	size_t result = fread( buffer , sizeof(char) , count , fp );
	string data;
	for( unsigned int i = 0 ; i < result ; i++ ){
		data += buffer[i];
	}
	Return( data );
	delete[] buffer;
}
void FileSystemLibrary::File_Read_u32( FILE* fp ){
	unsigned int data = 0;
	size_t result = fread( &data , sizeof(unsigned int) , 1 , fp );
	Return( data );
}
void FileSystemLibrary::File_Read_u16( FILE* fp ){
	unsigned short data = 0;
	size_t result = fread( &data , sizeof(unsigned short) , 1 , fp );
	Return( data );
}
void FileSystemLibrary::File_Read_u8( FILE* fp ){
	unsigned char data = 0;
	size_t result = fread( &data , sizeof(unsigned char) , 1 , fp );
	Return( data );
}
void FileSystemLibrary::File_Read_string( FILE* fp ){
	string data;
	char c = fgetc( fp );
	while( !feof( fp ) && c != '\0' ){
		data += c;
		c = fgetc( fp );				
	}
	Return( data );
}
void FileSystemLibrary::File_IsEof( FILE* fp ){
	int isEof = feof( fp );
	Return( isEof );
}
void FileSystemLibrary::File_Close( const unsigned int& filehandleid ){
	FILE_SYSTEM* file = getFileHandle( filehandleid );
	assert( file );
	assert( file->fp );
	fclose( file->fp );
	file->fp = NULL;
	file->handleId = 0;
}