#include "lib\vm\sencha_vm.h"
#include "vmplugin\VMFileSystem.h"
#include "vmplugin\VMSystem.h"
#include "memory_pool.h"
using namespace SenchaVM;
using namespace SenchaVM::Assembly;

int A(){
	printf( "A" );
	return 0;
}
int B(){
	printf( "B" );
	return 0;
}
int C(){
	printf( "C" );
	return 0;
}

int D(){
	printf( "D" );
	return 0;
}

void main(){
//	CLexcialReader lexer( new SenchaVM::LexcialReader( Sencha::Util::CStream( new Sencha::Util::FileStream( "sample/FizzBuzz.txt" ) ) ) );
//	CParser parser( new SenchaVM::Assembly::Parser( lexer->getResult() ) );
	int a = A() && B() || C() || D();
	printf( "\n" );
}
