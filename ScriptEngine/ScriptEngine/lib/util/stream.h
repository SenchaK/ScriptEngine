#pragma once
#include "binary.h"
#include <cassert>
#include <string>
#include <vector>
#include <memory>


namespace Sencha{
namespace Util {
using namespace std;
typedef unsigned char byte;


// �X�g���[�����
// ���̃N���X�̓f�[�^��1�o�C�g���ǂݐi�߂Ă����A�Ƃ����@�\��񋟂���B
// �������݋@�\���񋟂���Ă��邪�����͔C��
class Stream {
public :
	virtual ~Stream(){
	}

	/*
	 * 1�o�C�g��ǂݍ��݁A1�o�C�g�i�߂�
	 */
	virtual int getByte() = 0;
	/*
	 * ���ɐi�߂�̂��H
	 */
	virtual bool hasNext() = 0;
	/*
	 * �w��̈ʒu��1�o�C�g���擾����
	 */
	virtual int getByte( int position );
	/*
	 * �o�C�g�z��̈ʒu���w�肵�ď�������
	 */
	virtual void write( vector<byte>& contents , int startIndex , int size );
	/*
	 * �w��̃X�g���[���ʒu�Ɏw��̃f�[�^����������
	 */
	virtual void writePos( vector<byte>& contents , int position , int size );
	/* 
	 * �f�[�^����������
	 */
	virtual void write( byte value );
	/*
	 * �ʒu�擾
	 */
	virtual fpos_t position();

	/*
	 * �f�[�^�T�C�Y
	 */
	virtual fpos_t count();

	/*
	 * ���݂̈ʒu���Z�b�g
	 */
	virtual void position( fpos_t pos );
	/*
	 * �f�[�^����
	 */
	virtual void clear();

	/*
	 * �X�g���[������
	 */
	virtual void close();
};
typedef std::shared_ptr<Stream> CStream;


 
//�t�@�C���X�g���[�� 
class FileStream : public Stream {
public :
	enum Mode {
		Write       ,
		Read        ,
		ReadBinary  , 
		WriteBinary ,
	};
private :
	FILE* m_fp;
	fpos_t m_filesize;
private :
	void sizeinit( string& fileName );
public :
	FileStream( string fileName );
	FileStream( string fileName , FileStream::Mode mode );
	virtual ~FileStream();
	virtual int getByte() override;
	virtual int getByte( int position ) override;
	virtual long long count() override;
	virtual fpos_t position() override;
	virtual void position( fpos_t pos ) override;
	virtual bool hasNext() override;
	virtual void close() override;
};
FileStream* BinaryFileOpen( string fileName );
FileStream* TextFileOpen( string fileName );


// ������X�g���[��
class TextStream : public Stream {
private :
	string m_text;
	size_t m_index;
public  :
	TextStream( string text );
	virtual ~TextStream();
	virtual int getByte() override;
	virtual bool hasNext() override;
};

// �o�C�g�f�[�^�X�g���[��
// �ǂݏ��������\
class BinaryStream : public Stream {
private :
	vector<byte> m_binaryData;
	fpos_t m_index;
public  :
	BinaryStream();
	BinaryStream( vector<byte>& binaryData );
	virtual int getByte() override;
	virtual int getByte( int position ) override;
	virtual bool hasNext() override;
	virtual void write( byte value ) override;
	virtual fpos_t position() override;
	virtual void position( fpos_t pos ) override;
	virtual void write( vector<byte>& contents , int startIndex , int size ) override;
	virtual void writePos( vector<byte>& contents , int position , int size ) override;
	virtual void clear();
	virtual fpos_t count() override;
	virtual ~BinaryStream();
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
	fpos_t position();
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
	fpos_t count();
	CStream getStream();
};


typedef std::shared_ptr<BinaryWriter> CBinaryWriter;


} // namespace Util
} // namespace Sencha
