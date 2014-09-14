#pragma once

#include "../util/logger.h"
#include "vm_define.h"

#include "parser\vm_parser.h"
#include "vm_register.h"
#include "vm_assembler.h"
#include "assembly\vm_assemble_log.h"
#include "assembly\vm_assemble_io.h"


/*
	�y���t�@�����X�z

	*******************************************************************************************************************
	��class VMBuiltIn
		�g�ݍ��݊֐�(�\�[�X�ɒ�`�����֐�)���Ăяo���ۂɎg�p����B

	���g�p��
		VMBuiltIn* built_in = new VMBuiltIn();
		built_in->entryFunction( new VMBuiltInFunction( "Log" , built_in_function_Log ) );
		built_in->entryFunction( new VMBuiltInFunction( "ToString" , built_int_function_ToString ) );
	*******************************************************************************************************************

	*******************************************************************************************************************
	��class Lexer
		�����͂��s���Ƃ��Ɏg�p����B
		�R���X�g���N�^�ɃX�g���[����n���B
		�t�@�C������f�[�^��n�������ꍇ��FileStream�A
		�e�L�X�g����Ȃ�TextStream���g���B

	���g�p��
		Lexer* lexer = new Lexer( CStream( new FileStream( "sample/FizzBuzz.txt" ) ) );
	*******************************************************************************************************************


	*******************************************************************************************************************
	��class Parser
		�\����͂��s��
		���O�o�����������ꍇ��Log�N���X���A
		�g�ݍ��݊֐����Ăяo�������ꍇ��VMBuiltIn�N���X���R���X�g���N�^�����ɓn��
	���g�p��
		Parser* parser = new Parser( lexer , built_in , log );
		Parser* parser = new Parser( lexer , built_in );
		Parser* parser = new Parser( lexer );
	*******************************************************************************************************************


	*******************************************************************************************************************
	��class VMAssembleOutput
		�A�Z���u���f�[�^���o�C�i���t�@�C���ɂ��ĕۑ�����B
	���g�p��
		VMAssembleOutput output( parser , "parser_data.bin" );
	*******************************************************************************************************************


	*******************************************************************************************************************
	��class VMDriver
		�A�Z���u���f�[�^��n���Ď��s����B
		executeFunction()�Ăяo���֐�����n���Ď��s���邱�Ƃ��ł���B
	���g�p��
		VMDriver d( parser , built_in );
		d.executeFunction( "main" );
	*******************************************************************************************************************




	*******************************************************************************************************************
	��void VMAssembleTextFileLog( IAssembleReader* reader , const char* fileName )
		�A�Z���u���f�[�^�̃��O���w��̃t�@�C�����̃e�L�X�g�ō쐬
	���g�p��
		VMAssembleTextFileLog( parser , "parser_log.txt" );
	*******************************************************************************************************************
 */
namespace Sencha {
namespace VM {
using namespace Sencha::VM;
using namespace Sencha::VM::Assembly;

/*
 * �X�N���v�g�G���W������
 * ��ʂ�̋@�\��񋟂���B
 */
class SenchaVM{
private :
	Log* m_log;
	Lexer* m_lexer;
	IAssembleReader* m_reader;
	VMDriver* m_driver;
	VMBuiltIn* m_built_in;
private :
	void clear();
public :
	SenchaVM();
	~SenchaVM();
	void log_init( Log* logger );
	void cleanup();
	void compile_from_text( string text );
	void compile_from_file( const char* fileName );
	void compile_from_o_file( const char* objectFileName );
	void create_o_file( const char* objectFileName );
	void create_text_file_log( const char* fileName );
	void output_console_log();
	void define_function( string mappingName , void (*function)(VMDriver*) );
	void execute_function( string funcName );
	void execute();
};

} // namespace VM
} // namespace Sencha
