#include "util_stream.h"

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
/*virtual*/ 
int Stream::getByte( int position ){
	UTIL_ASSERT( 0 && "実装されていないメソッドが呼ばれました" );
	return 0;
}
/*virtual*/ 
void Stream::write( vector<byte> contents , int startIndex , int size ){
	UTIL_ASSERT( 0 && "実装されていないメソッドが呼ばれました" );
}
/*virtual*/
void Stream::writePos( vector<byte> contents , int position , int size ){
	UTIL_ASSERT( 0 && "実装されていないメソッドが呼ばれました" );
}
/*virtual*/ 
void Stream::write( byte value ){
	UTIL_ASSERT( 0 && "実装されていないメソッドが呼ばれました" );
}
/*virtual*/ 
int Stream::position(){
	UTIL_ASSERT( 0 && "実装されていないメソッドが呼ばれました" );
	return 0;
}
/*virtual*/ 
void Stream::position( int pos ){
	UTIL_ASSERT( 0 && "実装されていないメソッドが呼ばれました" );
}
/*virtual*/ 
void Stream::clear(){
	UTIL_ASSERT( 0 && "実装されていないメソッドが呼ばれました" );
}
/*virtual*/
int Stream::count(){
	UTIL_ASSERT( 0 && "実装されていないメソッドが呼ばれました" );
	return 0;
}
// *******************************************************************************



// *******************************************************************************
// class FileStream
// *******************************************************************************
FileStream::FileStream( string fileName ){
	m_fp = fopen( fileName.c_str() , "r" );
}

/*override*/
int FileStream::getByte(){
	if( !m_fp )        return EOF;
	if( feof( m_fp ) ) return EOF;
	return fgetc( m_fp );
}
/*override*/
bool FileStream::hasNext(){
	if( !m_fp )        return false;
	if( feof( m_fp ) ) return false;
	return true;
}

FileStream::~FileStream(){
	UTIL_PRINT( "FileStream Finish\n" );
	if( m_fp ){
		fclose( m_fp );
		m_fp = NULL;
	}
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
BinaryStream::BinaryStream( vector<byte> binaryData ){
	m_binaryData = binaryData;
	m_index = 0;
}
int BinaryStream::getByte(){
	if( m_index < 0 )                    return EOF;
	if( m_index >= m_binaryData.size() ) return EOF;
	return m_binaryData[m_index++];
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
int BinaryStream::position(){
	return m_index;
}
void BinaryStream::position( int pos ){
	m_index = pos;
}
void BinaryStream::write( vector<byte> contents , int startIndex , int size ){
	assert( startIndex >= 0 && startIndex < (int)contents.size() );
	for( int i = startIndex ; i < startIndex + size ; i++ ){
		m_binaryData.push_back( contents[i] );
	}
}
void BinaryStream::writePos( vector<byte> contents , int position , int size ){
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
int BinaryStream::count(){
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
int BinaryReader::position(){
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
int BinaryWriter::count(){
	return m_stream->count();
}
// *******************************************************************************



} // namespace Util
} // namespace Sencha

