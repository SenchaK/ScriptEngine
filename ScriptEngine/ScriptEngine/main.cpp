#include "lib\vm\sencha_vm.h"
#include <crtdbg.h>


using namespace Sencha;
using namespace Sencha::VM;
using namespace Sencha::VM::Assembly;
using namespace Sencha::Util;



void built_in_function_Log( VMDriver* driver ){
	string message = driver->popMemory().value_string;
	std::cout << message << std::endl;
}
void built_in_function_ToString( VMDriver* driver ){
	Memory& m = driver->popMemory();
	static char buf[512];
	sprintf_s<512>( buf , "%.f" , m.value );
	driver->Return( Memory( 0 , buf ) );
}


void main(){
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	SenchaVM* vm = new SenchaVM();
	vm->log_init( new ConsoleLog() );
	vm->define_function  ( "Log"      , built_in_function_Log      );
	vm->define_function  ( "ToString" , built_in_function_ToString );
	vm->compile_from_file( "sample/FizzBuzz.txt" );
	vm->execute_function ( "main" );
	delete vm;
}
