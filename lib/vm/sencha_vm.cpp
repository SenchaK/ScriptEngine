#include "sencha_vm.h"


namespace SenchaVM {

SenchaVM::Assembly::CVMMainRoutineDriver CompileFromText( string text ){
	SenchaVM::CLexcialReader lexer( new SenchaVM::LexcialReader( Sencha::Util::CStream( new Sencha::Util::TextStream( text ) ) ) );
	SenchaVM::Assembly::CParser parser( new SenchaVM::Assembly::Parser( lexer->getResult() ) );
	SenchaVM::Assembly::CVMMainRoutineDriver driver( new SenchaVM::Assembly::VMMainRoutineDriver() );
	driver->initialize( parser->getResult() , 2048 , 2048 , 32 );
	return driver;
}

SenchaVM::Assembly::CVMMainRoutineDriver OpenFromFile( string fileName ){
	SenchaVM::CLexcialReader lexer( new SenchaVM::LexcialReader( Sencha::Util::CStream( new Sencha::Util::FileStream( fileName ) ) ) );
	SenchaVM::Assembly::CParser parser( new SenchaVM::Assembly::Parser( lexer->getResult() ) );
	SenchaVM::Assembly::CVMMainRoutineDriver driver( new SenchaVM::Assembly::VMMainRoutineDriver() );
	driver->initialize( parser->getResult() , 2048 , 2048 , 32 );
	return driver;
}


} // namespace SenchaVM
