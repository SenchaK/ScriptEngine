#include "built_in_function_standard.h"

namespace Sencha {
namespace VM {
/* ************************************************************* *
 * Field
 * ************************************************************* */
static Log* log;
static event_handler evt;


/* ************************************************************* * 
 * Method
 * ************************************************************* */
static void print( const char* formatString , ...){
	va_list args;
	va_start( args , formatString );
	log->print( formatString , args );
	va_end( args );
}

/*
 * ログを出力する
 */
static void built_in_function_Log( VMDriver* driver ){
	string message = driver->popMemory().value_string;
//	std::cout << message << std::endl;
	print( "%s\n" , message.c_str() );
}

/*
 * 文字列変換
 */
static void built_in_function_ToString( VMDriver* driver ){
	Memory& m = driver->popMemory();
	static char buf[512];
	sprintf_s<512>( buf , "%.f" , m.value );
	driver->Return( Memory( 0 , buf ) );
}

/*
 * コルーチンを走らせる
 */
static void built_in_function_Invoke( VMDriver* driver ){
	Memory& m = driver->popMemory();
	driver->Invoke( m.value_string );
}

/*
 * 一時停止
 */
static void build_in_function_Yield( VMDriver* driver ){
	Memory& sleeptime = driver->popMemory();
	driver->Sleep( (int)sleeptime.value );
}

/*
 * 文字が空かチェック
 */
static void built_in_function_IsEmpty( VMDriver* driver ){
	Memory& m = driver->popMemory();
	if( m.value_string.compare( "" ) == 0 ){
		driver->Return( Memory( true , "" ) );
		return;
	}
	driver->Return( Memory( false , "" ) );
}

/*
 * 初期化
 */
static void initializer(){
	if( !log ){
		log = 
			new ConsoleLog();
			//new TextFileLog( "result.log" );
	}
}

/*
 * 終了処理
 */
static void finalizer(){
	if( log ){
		delete log;
		log = NULL;
	}
}

/*
 * 組み込み関数登録
 */
void built_in_function_standard( SenchaVM* vm ){
	evt.init = initializer;
	evt.finalize = finalizer;
	vm->define_function( "Log"      , built_in_function_Log      );
	vm->define_function( "ToString" , built_in_function_ToString );
	vm->define_function( "Invoke"   , built_in_function_Invoke   );
	vm->define_function( "Yield"    , build_in_function_Yield    );
	vm->define_function( "IsEmpty"  , built_in_function_IsEmpty  );
	vm->add_event( evt );
}

} // namespace Sencha
} // namespace VM
