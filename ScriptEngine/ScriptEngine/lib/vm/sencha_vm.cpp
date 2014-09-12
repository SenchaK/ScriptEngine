#include "sencha_vm.h"



namespace Sencha {
namespace VM {

//Sencha::Assembly::CVMMainRoutineDriver CompileFromText( string text ){
//	Sencha::CLexer lexer( new Sencha::Lexer( Sencha::Util::CStream( new Sencha::Util::TextStream( text ) ) ) );
//	Sencha::Assembly::CParser parser( new Sencha::Assembly::Parser( lexer->getResult() ) );
//	Sencha::Assembly::CVMMainRoutineDriver driver( new Sencha::Assembly::VMMainRoutineDriver() );
//	driver->initialize( parser->getResult() , 2048 , 2048 , 32 );
//	return driver;
//}

//Sencha::Assembly::CVMMainRoutineDriver OpenFromFile( string fileName ){
//	Sencha::CLexer lexer( new Sencha::Lexer( Sencha::Util::CStream( new Sencha::Util::FileStream( fileName ) ) ) );
//	Sencha::Assembly::CParser parser( new Sencha::Assembly::Parser( lexer->getResult() ) );
//	Sencha::Assembly::CVMMainRoutineDriver driver( new Sencha::Assembly::VMMainRoutineDriver() );
//	driver->initialize( parser->getResult() , 2048 , 2048 , 32 );
//	return driver;
//}

	namespace Assembly {
		/*
		 * �e�L�X�g�t�@�C���`���ɂ��ă��O����
		 */
		void VMAssembleTextFileLog( IAssembleReader* reader , const char* fileName ){
			VMAssembleLog log( reader , CLog( new TextFileLog( fileName ) ) );
		}

		/*
		 * �R���\�[���Ƀ��O�o��
		 */
		void VMAssembleConsoleLog( IAssembleReader* reader ){
			VMAssembleLog log( reader , CLog( new ConsoleLog() ) );
		}
	}

} // namespace VM
} // namespace Sencha
