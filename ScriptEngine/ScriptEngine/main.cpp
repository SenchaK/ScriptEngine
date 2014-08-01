#include "lib\vm\sencha_vm.h"
#include "vmplugin\VMFileSystem.h"
#include "vmplugin\VMSystem.h"

using namespace SenchaVM;
using namespace SenchaVM::Assembly;
void main(){
	CLexcialReader lexer( new SenchaVM::LexcialReader( Sencha::Util::CStream( new Sencha::Util::FileStream( "sample/FizzBuzz.txt" ) ) ) );
	CParser parser( new SenchaVM::Assembly::Parser( lexer->getResult() ) );
	//int ary[50];
	//int i[50];
	//int a= 1;
	//int b= 2;
	//int c= 5;
	//memset( ary , 0 , sizeof( ary ) );
	//memset( i , 0 , sizeof( i ) );
	//i[a-(b+c)] = ary[a*b+c];
}
