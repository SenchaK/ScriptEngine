#include "sencha_vm.h"



namespace SenchaVM {

//SenchaVM::Assembly::CVMMainRoutineDriver CompileFromText( string text ){
//	SenchaVM::CLexer lexer( new SenchaVM::Lexer( Sencha::Util::CStream( new Sencha::Util::TextStream( text ) ) ) );
//	SenchaVM::Assembly::CParser parser( new SenchaVM::Assembly::Parser( lexer->getResult() ) );
//	SenchaVM::Assembly::CVMMainRoutineDriver driver( new SenchaVM::Assembly::VMMainRoutineDriver() );
//	driver->initialize( parser->getResult() , 2048 , 2048 , 32 );
//	return driver;
//}

//SenchaVM::Assembly::CVMMainRoutineDriver OpenFromFile( string fileName ){
//	SenchaVM::CLexer lexer( new SenchaVM::Lexer( Sencha::Util::CStream( new Sencha::Util::FileStream( fileName ) ) ) );
//	SenchaVM::Assembly::CParser parser( new SenchaVM::Assembly::Parser( lexer->getResult() ) );
//	SenchaVM::Assembly::CVMMainRoutineDriver driver( new SenchaVM::Assembly::VMMainRoutineDriver() );
//	driver->initialize( parser->getResult() , 2048 , 2048 , 32 );
//	return driver;
//}

	namespace Assembly {
		/*
		 * テキストファイル形式にしてログ生成
		 */
		void VMAssembleTextFileLog( IAssembleReader* reader , const char* fileName ){
			VMAssembleLog log( reader , CLog( new TextFileLog( fileName ) ) );
		}

		/*
		 * コンソールにログ出力
		 */
		void VMAssembleConsoleLog( IAssembleReader* reader ){
			VMAssembleLog log( reader , CLog( new ConsoleLog() ) );
		}
	}

} // namespace SenchaVM
