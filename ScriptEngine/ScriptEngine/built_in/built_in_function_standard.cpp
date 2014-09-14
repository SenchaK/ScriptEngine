#include "built_in_function_standard.h"

namespace Sencha {
namespace VM {

/*
 * ログを出力する
 */
void built_in_function_Log( VMDriver* driver ){
	string message = driver->popMemory().value_string;
	std::cout << message << std::endl;
}

/*
 * 文字列変換
 */
void built_in_function_ToString( VMDriver* driver ){
	Memory& m = driver->popMemory();
	static char buf[512];
	sprintf_s<512>( buf , "%.f" , m.value );
	driver->Return( Memory( 0 , buf ) );
}


/*
 * 組み込み関数登録
 */
void built_in_function_standard( SenchaVM* vm ){
	vm->define_function  ( "Log"      , built_in_function_Log      );
	vm->define_function  ( "ToString" , built_in_function_ToString );
}

} // namespace Sencha
} // namespace VM
