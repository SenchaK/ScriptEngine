#pragma once

#include "../util/logger.h"
#include "vm_define.h"

#include "parser\vm_parser.h"
#include "vm_register.h"
#include "vm_assembler.h"
#include "assembly\vm_assemble_log.h"
#include "assembly\vm_assemble_io.h"


/*
	【リファレンス】

	*******************************************************************************************************************
	■class VMBuiltIn
		組み込み関数(ソースに定義した関数)を呼び出す際に使用する。

	●使用例
		VMBuiltIn* built_in = new VMBuiltIn();
		built_in->entryFunction( new VMBuiltInFunction( "Log" , built_in_function_Log ) );
		built_in->entryFunction( new VMBuiltInFunction( "ToString" , built_int_function_ToString ) );
	*******************************************************************************************************************

	*******************************************************************************************************************
	■class Lexer
		字句解析を行うときに使用する。
		コンストラクタにストリームを渡す。
		ファイルからデータを渡したい場合はFileStream、
		テキストからならTextStreamを使す。

	●使用例
		Lexer* lexer = new Lexer( CStream( new FileStream( "sample/FizzBuzz.txt" ) ) );
	*******************************************************************************************************************


	*******************************************************************************************************************
	■class Parser
		構文解析を行う
		ログ出しをしたい場合はLogクラスを、
		組み込み関数を呼び出したい場合はVMBuiltInクラスをコンストラクタ引数に渡す
	●使用例
		Parser* parser = new Parser( lexer , built_in , log );
		Parser* parser = new Parser( lexer , built_in );
		Parser* parser = new Parser( lexer );
	*******************************************************************************************************************


	*******************************************************************************************************************
	■class VMAssembleOutput
		アセンブルデータをバイナリファイルにして保存する。
	●使用例
		VMAssembleOutput output( parser , "parser_data.bin" );
	*******************************************************************************************************************


	*******************************************************************************************************************
	■class VMDriver
		アセンブルデータを渡して実行する。
		executeFunction()呼び出す関数名を渡して実行することができる。
	●使用例
		VMDriver d( parser , built_in );
		d.executeFunction( "main" );
	*******************************************************************************************************************




	*******************************************************************************************************************
	■void VMAssembleTextFileLog( IAssembleReader* reader , const char* fileName )
		アセンブリデータのログを指定のファイル名のテキストで作成
	●使用例
		VMAssembleTextFileLog( parser , "parser_log.txt" );
	*******************************************************************************************************************
 */
namespace Sencha {
namespace VM{
namespace Assembly {
	void VMAssembleTextFileLog( IAssembleReader* reader , const char* fileName );
	void VMAssembleConsoleLog( IAssembleReader* reader );
}
}
}
