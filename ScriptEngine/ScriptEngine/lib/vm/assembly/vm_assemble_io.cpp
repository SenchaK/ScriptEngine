#include "vm_assemble_io.h"


namespace SenchaVM{
namespace Assembly{

/*
 * ��͌��ʂ��o�C�i���`���̃t�@�C���ɏo�͂���B
 * �����R�[�h��ʃt�@�C���ɂ���p�r�Ŏg�p����B
 *
 * �e�A�Z���u���͈ȉ��̃t�H�[�}�b�g�`���ŕۑ������
 * [AssemblyName : string           ] �A�Z���u���̖��O
 * [StackFrame   : u32              ] �X�^�b�N�t���[��
 * [Address      : u32              ] �֐��A�h���X
 * [Args         : u32              ] �֐��p�����[�^��
 * [CodeSize     : u32              ] �R�[�h�̈�T�C�Y
 * [Code         : byte[ContentSize]] �R�[�h�̈�
 */
VMAssembleOutput::VMAssembleOutput( IAssembleReader* reader , const char* fileName ){
	BinaryWriter bw;
	int index = 0;
	AsmInfo* assemblyInfo = reader->getAssembly(0);
	while( assemblyInfo ){
		bw.writeString( assemblyInfo->name() );
		bw.writeUInt32( assemblyInfo->stackFrame() );
		bw.writeUInt32( assemblyInfo->addr() );
		bw.writeUInt32( assemblyInfo->Args() );
		bw.writeUInt32( assemblyInfo->CodeSize() );

		for( size_t i = 0 ; i < assemblyInfo->CodeSize() ; i++ ){
			bw.write( assemblyInfo->getCommand(i) );
		}
		assemblyInfo = reader->getAssembly(++index);
	}
	CStream stream = bw.getStream();
	FILE* fp = NULL;
	fopen_s( &fp , fileName , "wb" );
	assert( fp );
	while( stream->hasNext() ){
		unsigned char content[1];
		content[0] = static_cast<unsigned char>( stream->getByte() );
		fwrite( content , sizeof( unsigned char ) , sizeof( content ) , fp );
	}
	fclose( fp );
}


VMAssembleInput::VMAssembleInput( CStream stream ){
	assert( stream.get() );
	BinaryReader reader( stream );
	this->m_asm = new VMAssembleCollection();
	while( reader.hasNext() ){
		vector<unsigned char> Code;
		string AssemblyName = reader.ToString();
		size_t StackFrame = reader.ToUInt32();
		size_t Address = reader.ToUInt32();
		size_t Args = reader.ToUInt32();
		size_t CodeSize = reader.ToUInt32();

		if( !reader.hasNext() ){
			break;
		}
		for( size_t i = 0 ; i < CodeSize ; i++ ){
			Code.push_back( reader.getByte() );
		}
		AsmInfo* asmInfo = new AsmInfo();
		asmInfo->setName( AssemblyName );
		asmInfo->setStackFrame( StackFrame );
		asmInfo->setAddress( Address );
		asmInfo->setArgs( Args );
		asmInfo->setBytes( CStream( new BinaryStream( Code ) ) );
		this->m_asm->entryAssembly( asmInfo );
	}
}

} // Assembly
} // SenchaVM