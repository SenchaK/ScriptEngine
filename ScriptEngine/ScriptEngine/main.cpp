#include "lib\vm\sencha_vm.h"
#include "built_in\built_in_function_standard.h"
#include <crtdbg.h>
using namespace Sencha;
using namespace Sencha::VM;
using namespace Sencha::VM::Assembly;
using namespace Sencha::Util;

void main(){
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	SenchaVM* vm = new SenchaVM();
	vm->log_init( new ConsoleLog() );
	built_in_function_standard( vm );
	vm->compile_from_file( "sample/Fibonacci.snc" );
	vm->create_text_file_log( "parser_log.txt" );
	vm->execute_function ( "main" );
	delete vm;
}
