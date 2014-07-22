#pragma once

#include "../lib/vm/sencha_vm.h"
using namespace std;

struct FILE_SYSTEM {
	FILE* fp;
	int handleId;
};

class FileSystemLibrary : public SenchaVM::Assembly::VMSystemCallService {
private :
	int serial;
	FILE_SYSTEM fileSystem[5];
private :
	int findEmptyFileHandle();
	FILE_SYSTEM* getFileHandle( int handleid );
public :
	FileSystemLibrary();
public :
	virtual void callFunction( string funcName );
private :
	/*
	 * スクリプトへの提供関数一覧
	 * 引数はスクリプト側が渡すべき値と一対にする
	 */
	void File_Open        ( const string& fileName , const string& mode );
	void File_Write       ( FILE* fp , const string& value );
	void File_Write_u32   ( FILE* fp , const unsigned long&  u32value );
	void File_Write_u16   ( FILE* fp , const unsigned short& u16value );
	void File_Write_u8    ( FILE* fp , const unsigned char&  u8value  );
	void File_Write_string( FILE* fp , const string&  string_value );
	void File_GetSize     ( FILE* fp );
	void File_GetChar     ( FILE* fp );
	void File_ReadBytes   ( FILE* fp , const int count );
	void File_Read_u32    ( FILE* fp );
	void File_Read_u16    ( FILE* fp );
	void File_Read_u8     ( FILE* fp );
	void File_Read_string ( FILE* fp );
	void File_IsEof       ( FILE* fp );
	void File_Close       ( const unsigned int& filehandleid );
};
