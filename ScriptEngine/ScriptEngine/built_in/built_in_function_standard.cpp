#include "built_in_function_standard.h"

namespace Sencha {
namespace VM {

/*
 * ƒƒO‚ðo—Í‚·‚é
 */
static void built_in_function_Log( VMDriver* driver ){
	string message = driver->popMemory().value_string;
	std::cout << message << std::endl;
}

/*
 * •¶Žš—ñ•ÏŠ·
 */
static void built_in_function_ToString( VMDriver* driver ){
	Memory& m = driver->popMemory();
	static char buf[512];
	sprintf_s<512>( buf , "%.f" , m.value );
	driver->Return( Memory( 0 , buf ) );
}

/*
 * ƒRƒ‹[ƒ`ƒ“‚ð‘–‚ç‚¹‚é
 */
static void built_in_function_Invoke( VMDriver* driver ){
	Memory& m = driver->popMemory();
	driver->Invoke( m.value_string );
}

/*
 * ˆêŽž’âŽ~
 */
static void build_in_function_Yield( VMDriver* driver ){
	Memory& sleeptime = driver->popMemory();
	driver->Sleep( (int)sleeptime.value );
}

/*
 * ‘g‚Ýž‚ÝŠÖ”“o˜^
 */
void built_in_function_standard( SenchaVM* vm ){
	vm->define_function  ( "Log"      , built_in_function_Log      );
	vm->define_function  ( "ToString" , built_in_function_ToString );
	vm->define_function  ( "Invoke"   , built_in_function_Invoke   );
	vm->define_function  ( "Yield"    , build_in_function_Yield    );
}

} // namespace Sencha
} // namespace VM
