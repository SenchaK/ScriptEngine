#include "lib\vm\sencha_vm.h"
#include "vmplugin\VMFileSystem.h"
#include "vmplugin\VMSystem.h"
#include "memory_pool.h"
using namespace SenchaVM;
using namespace SenchaVM::Assembly;
using namespace Sencha::Util;


void main(){
	Lexer* lexer = new Lexer( CStream( new FileStream( "sample/FizzBuzz.txt" ) ) );
	Parser* parser = new Parser( lexer );

	delete lexer;
	delete parser;
}
