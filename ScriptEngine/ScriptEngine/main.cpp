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
	vm->compile_from_file( "sample/FizzBuzz.snc" );
	vm->create_text_file_log( "parser_log.txt" );
	vm->execute_function ( "main" );
	vm->on_update();

	printf( "0:%.f\n" , vm->L( 0 )->value );
	printf( "1:%.f\n" , vm->L( 1 )->value );
	printf( "2:%.f\n" , vm->L( 2 )->value );
	printf( "3:%.f\n" , vm->L( 3 )->value );
	printf( "4:%.f\n" , vm->L( 4 )->value );
	printf( "5:%.f\n" , vm->L( 5 )->value );
	printf( "6:%.f\n" , vm->L( 6 )->value );
	printf( "bp:%d , sp:%d , push:%d \n" , vm->BaseP() , vm->SP() , vm->Push() );
	delete vm;
}
