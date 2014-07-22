#pragma once

#include "../lib/vm/sencha_vm.h"
using namespace std;

class VMSystem : public SenchaVM::Assembly::VMSystemCallService {
public :
	VMSystem();
public :
	virtual void callFunction( string funcName );
private :
	/*
	 * スクリプトへの提供関数一覧
	 * 引数はスクリプト側が渡すべき値と一対にする
	 */
	void Log        ( const string& message );
	void sleep      ( const int sleeptime );
	void str_len    ( const string& string_value );
	void str_replace( const string& string_value , string from , string to );
	void str_getc   ( const string& string_value , const int& index );
};
