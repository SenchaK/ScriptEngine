#include "lib\vm\sencha_vm.h"
#include "vmplugin\VMFileSystem.h"
#include "vmplugin\VMSystem.h"
#include "memory_pool.h"
using namespace SenchaVM;
using namespace SenchaVM::Assembly;

void main(){	
	CLexcialReader lexer( new SenchaVM::LexcialReader( Sencha::Util::CStream( new Sencha::Util::FileStream( "sample/FizzBuzz.txt" ) ) ) );
	CParser parser( new SenchaVM::Assembly::Parser( lexer->getResult() ) );
}
