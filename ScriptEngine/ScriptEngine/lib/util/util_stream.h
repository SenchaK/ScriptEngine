#pragma once
#include "util_binary.h"
#include <cassert>
#include <string>
#include <vector>
#include <memory>


namespace Sencha{
namespace Util {
using namespace std;
typedef unsigned char byte;


// ストリーム基底
// このクラスはデータを1バイトずつ読み進めていく、という機能を提供する。
// 書き込み機能も提供されているが実装は任意
class Stream {
public :
	virtual int getByte() = 0;
	virtual bool hasNext() = 0;
	virtual int getByte( int position );
	virtual void write( vector<byte> contents , int startIndex , int size );
	virtual void writePos( vector<byte> contents , int position , int size );
	virtual void write( byte value );
	virtual int position();
	virtual int count();
	virtual void position( int pos );
	virtual void clear();
};
typedef std::shared_ptr<Stream> CStream;


 
//ファイルストリーム 
class FileStream : public Stream {
private :
	FILE* m_fp;
public :
	FileStream( string fileName );
	~FileStream();
	virtual int getByte() override;
	virtual bool hasNext() override;
	void close();
};


// 文字列ストリーム
class TextStream : public Stream {
private :
	string m_text;
	size_t m_index;
public  :
	TextStream( string text );
	~TextStream();
	virtual int getByte() override;
	virtual bool hasNext() override;
};

// バイトデータストリーム
// 読み書き両方可能
class BinaryStream : public Stream {
private :
	vector<byte> m_binaryData;
	size_t m_index;
public  :
	BinaryStream();
	BinaryStream( vector<byte> binaryData );
	virtual int getByte() override;
	virtual int getByte( int position ) override;
	virtual bool hasNext() override;
	virtual void write( byte value ) override;
	virtual int position() override;
	virtual void position( int pos ) override;
	virtual void write( vector<byte> contents , int startIndex , int size ) override;
	virtual void writePos( vector<byte> contents , int position , int size ) override;
	virtual void clear();
	virtual int count() override;
	~BinaryStream();
};



class TextReader {
private :
	string m_text;
public :
	TextReader( CStream stream );
	string getResult();
	~TextReader();
};
typedef std::shared_ptr<TextReader> CTextReader;


class BinaryReader {
private :
	CStream m_stream;
public  :
	BinaryReader( CStream stream );
	bool hasNext();
	int getByte();
	int getByte( int position );
	void position( int pos );
	int position();
	signed int ToInt32();
	signed short int ToInt16();
	unsigned int ToUInt32();
	unsigned short int ToUInt16();
	float ToSingle();
	double ToDouble();
	string ToString();
};
typedef std::shared_ptr<BinaryReader> CBinaryReader;


class BinaryWriter {
private :
	CStream m_stream;
public  :
	BinaryWriter();
	BinaryWriter( CStream stream );
	~BinaryWriter();
	void write( byte value );
	void writeInt32( signed long int value );
	void writeInt32( signed long int value , int position );

	void writeInt16( signed short int value );
	void writeUInt32( unsigned long int value );
	void writeUInt16( unsigned short int value );
	void writeSingle( float value );
	void writeDouble( double value );
	void writeString( string value );
	void clear();
	void append( BinaryWriter& w );
	int count();
	CStream getStream();
};
typedef std::shared_ptr<BinaryWriter> CBinaryWriter;


} // namespace Util
} // namespace Sencha
