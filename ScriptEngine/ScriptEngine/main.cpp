#include "lib\vm\sencha_vm.h"
#include "vmplugin\VMFileSystem.h"
#include "vmplugin\VMSystem.h"
#include "lib\vm\assembly\vmassemble_reader.h"
#include "memory_pool.h"

using namespace SenchaVM;
using namespace SenchaVM::Assembly;
using namespace Sencha::Util;


void main(){
	Log* log = new ConsoleLog();
	Lexer* lexer = new Lexer( CStream( new FileStream( "sample/FizzBuzz.txt" ) ) );
	Parser* parser = new Parser( lexer );

	VMTextFileWriter writer( parser , "dump.txt" );
	delete lexer;
	delete parser;
	delete log;
}
