#include "stream.h"
#include "exception\exception.h"
#include <sys\stat.h>

#ifdef UTIL_DEBUG
	#define UTIL_ASSERT assert
	#define UTIL_PRINT printf
#else
	#define UTIL_ASSERT(...)
	#define UTIL_PRINT(...)
#endif

#pragma warning ( disable : 4996 )


namespace Sencha{
namespace Util {
// *******************************************************************************
// class Stream
// *******************************************************************************
/* virtual */ 
int Stream::getByte( int position ){
	throw NotImplementException( "実装されていないメソッドが呼ばれました int Stream::getByte( int position )" );
}
/* virtual */ 
void Stream::write( vector<byte>& contents , int startIndex , int size ){
	throw NotImplementException( "実装されていないメソッドが呼ばれました void Stream::write( vector<byte> contents , int startIndex , int size )" );
}
/* virtual */
void Stream::writePos( vector<byte>& contents , int position , int size ){
	throw NotImplementException( "実装されていないメソッドが呼ばれました void Stream::writePos( vector<byte> contents , int position , int size )" );
}
/* virtual */ 
void Stream::write( byte value ){
	throw NotImplementException( "実装されていないメソッドが呼ばれました void Stream::write( byte value )" );
}
/* virtual */ 
fpos_t Stream::position(){
	throw NotImplementException( "実装されていないメソッドが呼ばれました int Stream::position()" );
}
/* virtual */ 
void Stream::position( fpos_t pos ){
	throw NotImplementException( "実装されていないメソッドが呼ばれました void Stream::position( int pos )" );
}
/* virtual */ 
void Stream::clear(){
	throw NotImplementException( "実装されていないメソッドが呼ばれました void Stream::clear()" );
}
/* virtual */
fpos_t Stream::count(){
	throw NotImplementException( "実装されていないメソッドが呼ばれました int Stream::count()" );
}
/* virtual */
void Stream::close(){
	throw NotImplementException( "実装されていないメソッドが呼ばれました void Stream::close()" );
}

// *******************************************************************************



// *******************************************************************************
// class FileStream
// *******************************************************************************
static const char* GetFileMode( FileStream::Mode mode ){
	switch( mode ){
		case FileStream::Write       : return "w";
		case FileStream::Read        : return "r"; 
		case FileStream::WriteBinary : return "wb";
		case FileStream::ReadBinary  : return "rb";
	}
	throw Exception( "不明なファイルモード" );
}

FileStream* BinaryFileOpen( string fileName ){
	return new FileStream( fileName , FileStream::ReadBinary );
}

FileStream* TextFileOpen( string fileName ){
	return new FileStream( fileName );
}

FileStream::FileStream( string fileName ){
	this->sizeinit( fileName );
	this->m_fp = fopen( fileName.c_str() , GetFileMode( FileStream::Read ) );
}

FileStream::FileStream( string fileName , FileStream::Mode mode ){
	this->sizeinit( fileName );
	this->m_fp = fopen( fileName.c_str() , GetFileMode( mode ) );
}

void FileStream::sizeinit( string& fileName ){
	fpos_t fsize;
	FILE *fp = fopen( fileName.c_str() , GetFileMode( FileStream::ReadBinary ) );
	if( fp ){
		fseek( fp , 0 , SEEK_END ); 
		fgetpos( fp , &fsize ); 
		fclose( fp );
	}
	this->m_filesize = fsize;
}

/* override */
int FileStream::getByte(){
	if( !m_fp )        return EOF;
	if( feof( m_fp ) ) return EOF;
	return fgetc( m_fp );
}

/* override */
int FileStream::getByte( int p ){
	fpos_t current = this->position();
	this->position( p );
	int c = this->getByte();
	this->position( current );
	return c;
}

/* override */
bool FileStream::hasNext(){
	if( !m_fp ){
		return false;
	}
	if( feof( m_fp ) ){
		this->close();
		return false;
	}
	return true;
}

/* override */
fpos_t FileStream::position(){
	if( !this->m_fp ){
		return 0;
	}
	fpos_t pos;
	fgetpos( this->m_fp , &pos );
	return pos;
}

/* override */
void FileStream::position( fpos_t pos ){
	fpos_t p = pos;
	fsetpos( this->m_fp , &p );
}

/* override */
fpos_t FileStream::count(){
	return this->m_filesize;
}

/* override */
void FileStream::close(){
	if( m_fp ){
		fclose( m_fp );
		m_fp = NULL;
	}
}

FileStream::~FileStream(){
	UTIL_PRINT( "FileStream Finish\n" );
	this->close();
}
// *******************************************************************************



// *******************************************************************************
// class TextStream
// *******************************************************************************
TextStream::TextStream( string text ){
	m_text = text;
	m_index = 0;
}
/*override*/
int TextStream::getByte(){
	if( m_index < 0 )                return EOF;
	if( m_index >= m_text.length() ) return EOF;
	return m_text[m_index++];
}
/*override*/
bool TextStream::hasNext(){
	if( m_index >= m_text.length() ) return false;
	return true;
}
TextStream::~TextStream(){
	UTIL_PRINT( "TextStream Finish\n" );
}
// *******************************************************************************





// *******************************************************************************
// class BinaryStream
// *******************************************************************************
BinaryStream::BinaryStream(){
	m_index = 0;
}
BinaryStream::BinaryStream( vector<byte>& binaryData ){
	m_binaryData = binaryData;
	m_index = 0;
}
int BinaryStream::getByte(){
	if( m_index < 0 )                    return EOF;
	if( m_index >= m_binaryData.size() ) return EOF;
	unsigned int index = (unsigned int)m_index;
	m_index++;
	return m_binaryData[index];
}
int BinaryStream::getByte( int position ){
	if( position < 0 )                         return EOF;
	if( position >= (int)m_binaryData.size() ) return EOF;
	return m_binaryData[position];
}
bool BinaryStream::hasNext(){
	if( m_index < 0 )                    return false;
	if( m_index >= m_binaryData.size() ) return false;
	return true;
}
void BinaryStream::write( byte value ){
	m_binaryData.push_back( value );
}
fpos_t BinaryStream::position(){
	return m_index;
}
void BinaryStream::position( fpos_t pos ){
	m_index = pos;
}
void BinaryStream::write( vector<byte>& contents , int startIndex , int size ){
	assert( startIndex >= 0 && startIndex < (int)contents.size() );
	for( int i = startIndex ; i < startIndex + size ; i++ ){
		m_binaryData.push_back( contents[i] );
	}
}
void BinaryStream::writePos( vector<byte>& contents , int position , int size ){
	UTIL_ASSERT( position >= 0 && ( position + size ) <= (int)m_binaryData.size() );
	for( int pos = position ; pos < position + size ; pos++ ){
		int index = pos - position;
		m_binaryData[pos] = contents[index];
	}
}
void BinaryStream::clear(){
	m_binaryData.clear();
	m_index = 0;
}
fpos_t BinaryStream::count(){
	return (int)m_binaryData.size();
}
BinaryStream::~BinaryStream(){
	UTIL_PRINT( "BinaryStream Finish\n" );
}
// *******************************************************************************





// *******************************************************************************
// class TextReader
// *******************************************************************************
TextReader::TextReader( CStream stream ){
	m_text = "";
	char c = 0;

	while( true ){
		c = stream->getByte();
		if( c == EOF ){
			break;
		}
		m_text += c;
	}
}
string TextReader::getResult(){
	return m_text;
}
TextReader::~TextReader(){
	UTIL_PRINT( "Reader Finish\n" );
}




// *******************************************************************************
// class BinaryReader
// *******************************************************************************
BinaryReader::BinaryReader( CStream stream ){
	m_stream = stream;
}
bool BinaryReader::hasNext(){
	return m_stream->hasNext();
}
int BinaryReader::getByte(){
	return m_stream->getByte();
}
int BinaryReader::getByte( int position ){
	return m_stream->getByte( position );
}
void BinaryReader::position( int pos ){
	m_stream->position( pos );
}
fpos_t BinaryReader::position(){
	return m_stream->position();
}
signed int BinaryReader::ToInt32(){
	vector<byte> contents;
	contents.push_back( getByte() );
	contents.push_back( getByte() );
	contents.push_back( getByte() );
	contents.push_back( getByte() );
	return Bin::byteToInt32( contents , 0 );
}
signed short int BinaryReader::ToInt16(){
	vector<byte> contents;
	contents.push_back( getByte() );
	contents.push_back( getByte() );
	return Bin::byteToInt16( contents , 0 );
}
unsigned int BinaryReader::ToUInt32(){
	vector<byte> contents;
	contents.push_back( getByte() );
	contents.push_back( getByte() );
	contents.push_back( getByte() );
	contents.push_back( getByte() );
	return (unsigned int)Bin::byteToInt32( contents , 0 );
}
unsigned short int BinaryReader::ToUInt16(){
	vector<byte> contents;
	contents.push_back( getByte() );
	contents.push_back( getByte() );
	return Bin::byteToUInt16( contents , 0 );
}
float BinaryReader::ToSingle(){
	vector<byte> contents;
	contents.push_back( getByte() );
	contents.push_back( getByte() );
	contents.push_back( getByte() );
	contents.push_back( getByte() );
	return Bin::byteToSingle( contents , 0 );
}
double BinaryReader::ToDouble(){
	vector<byte> contents;
	contents.push_back( getByte() );
	contents.push_back( getByte() );
	contents.push_back( getByte() );
	contents.push_back( getByte() );
	contents.push_back( getByte() );
	contents.push_back( getByte() );
	contents.push_back( getByte() );
	contents.push_back( getByte() );
	return Bin::byteToDouble( contents , 0 );
}
string BinaryReader::ToString(){
	vector<byte> contents;
	while( hasNext() ){
		byte content = getByte();
		contents.push_back( content );
		if( content == 0 ){
			break;
		}
	}
	return Bin::byteToString( contents , 0 );
}
// *******************************************************************************




// *******************************************************************************
// class BinaryWriter
// *******************************************************************************
BinaryWriter::BinaryWriter(){
	m_stream = CStream( new BinaryStream() );
}
BinaryWriter::BinaryWriter( CStream stream ){
	m_stream = stream;
}
BinaryWriter::~BinaryWriter(){
	UTIL_PRINT( "BinaryWriter Finish\n" );
}

void BinaryWriter::write( byte value ){
	m_stream->write( value );
}
void BinaryWriter::writeInt32( signed long int value ){
	m_stream->write( Bin::int32ToByte( value ) , 0 , 4 );
}
void BinaryWriter::writeInt32( signed long int value , int position ){
	m_stream->writePos( Bin::int32ToByte( value ) , position , 4 );
}
void BinaryWriter::writeInt16( signed short int value ){
	m_stream->write( Bin::int16ToByte( value ) , 0 , 2 );
}
void BinaryWriter::writeUInt32( unsigned long int value ){
	m_stream->write( Bin::uint32ToByte( value ) , 0 , 4 );
}
void BinaryWriter::writeUInt16( unsigned short int value ){
	m_stream->write( Bin::uint16ToByte( value ) , 0 , 2 );
}
void BinaryWriter::writeSingle( float value ){
	m_stream->write( Bin::singleToByte( value ) , 0 , 4 );
}
void BinaryWriter::writeDouble( double value ){
	m_stream->write( Bin::doubleToByte( value ) , 0 , 8 );
}
void BinaryWriter::writeString( string value ){
	m_stream->write( Bin::stringToByte( value ) , 0 , value.length() );
	m_stream->write( 0 );
}
CStream BinaryWriter::getStream(){
	return m_stream;
}
void BinaryWriter::clear(){
	m_stream->clear();
}
fpos_t BinaryWriter::count(){
	return m_stream->count();
}
void BinaryWriter::append( BinaryWriter& w ){
	while( w.m_stream->hasNext() ){
		m_stream->write( w.m_stream->getByte() );
	}
}
// *******************************************************************************



} // namespace Util
} // namespace Sencha

