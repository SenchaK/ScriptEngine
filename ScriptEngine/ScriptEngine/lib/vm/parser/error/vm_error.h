#pragma once
#include "../../vm_define.h"


namespace Sencha {
namespace VM {
namespace Assembly {
using namespace std;

enum ERR_ID {
	ERROR_C2065 , // "symbolName" : 定義されていない識別子です。
	ERROR_C2059 , // 構文エラー : 's'
	ERROR_C2143 , // 構文エラー : ']' が ';' の前にありません。
};

/*
 * エラー列挙型の拡張機能
 */
class ERR_ID_EX{
public :
	/*
	 * ＩＤを文字列にする
	 */
	static string toString( ERR_ID value );
};

/*
 * エラー時のパラメータ基底
 * エラーの種類によって欲しいパラメータが異なるので抽象的にオブジェクトのみ用意する
 */
class VMErrorParameter {
public :
	ERR_ID errId;
public :
	VMErrorParameter( ERR_ID errid ){
		this->errId = errid;
	}
	virtual ~VMErrorParameter(){
	}
};

class ERROR_INFO_C2065 : public VMErrorParameter {
public :
	string symbolName;
	ERROR_INFO_C2065( const string& symbolName ) : VMErrorParameter( ERROR_C2065 ){
		this->symbolName = symbolName;
	}
};

class ERROR_INFO_C2143 : public VMErrorParameter {
public :
	int tokenType;
	ERROR_INFO_C2143( int tokenType ) : VMErrorParameter( ERROR_C2143 ){
		this->tokenType = tokenType;
	}
};

class ERROR_INFO_C2059 : public VMErrorParameter {
public :
	string s;
	ERROR_INFO_C2059( string s ) : VMErrorParameter( ERROR_C2059 ){
		this->s = s;
	}
};

/*
 * 仮想機械関係でのエラー処理など
 */
class VMError {
protected :
	string m_message;
	VMErrorParameter* m_param;
public :
	VMError( VMErrorParameter* param );
	virtual ~VMError();
	const string& getMessage(){
		return m_message;
	}
};


} // Assembly
} // VM
} // Sencha