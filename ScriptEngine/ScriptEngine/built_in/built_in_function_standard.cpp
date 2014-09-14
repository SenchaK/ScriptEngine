#include "built_in_function_standard.h"

namespace Sencha {
namespace VM {

/*
 * ƒƒO‚ðo—Í‚·‚é
 */
void built_in_function_Log( VMDriver* driver ){
	string message = driver->popMemory().value_string;
	std::cout << message << std::endl;
}

/*
 * •¶Žš—ñ•ÏŠ·
 */
void built_in_function_ToString( VMDriver* driver ){
	Memory& m = driver->popMemory();
	static char buf[512];
	sprintf_s<512>( buf , "%.f" , m.value );
	driver->Return( Memory( 0 , buf ) );
}


/*
 * ‘g‚Ýž‚ÝŠÖ”“o˜^
 */
void built_in_function_standard( SenchaVM* vm ){
	vm->define_function  ( "Log"      , built_in_function_Log      );
	vm->define_function  ( "ToString" , built_in_function_ToString );
}

} // namespace Sencha
} // namespace VM
