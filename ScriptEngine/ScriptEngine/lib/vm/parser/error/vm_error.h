#pragma once
#include "../../vm_define.h"


namespace Sencha {
namespace VM {
namespace Assembly {
using namespace std;

enum ERR_ID {
	// 1000番
	// 式評価エラー
	ERROR_1001 , // 参照型に代入以外のデータ命令をかけている
	ERROR_1002 , // 左辺が参照型である場合、右辺は対応するデータだけでなければいけない

	// 2000番
	// シンタックスエラー
	ERROR_2001 , // '['に対応する記号']'が見つからない
	ERROR_2002 , // '('に対応する記号')'が見つからない
	ERROR_2003 , // '{'に対応する記号'}'が見つからない

	ERROR_C2065 , // "symbolName" : 定義されていない識別子です。
	ERROR_C2059 , // 構文エラー : 's'
	ERROR_C2143 , // 構文エラー : ']' が ';' の前にありません。


	// 3000番
	// 宣言エラー
	ERROR_3001 , // 構造体内部で関数宣言された
	ERROR_3002 , // 同じ名前の関数が既に登録済みである

	// 4000番
	// シンボルエラー
	ERROR_4001 , // 不明なシンボルタイプ

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

/* 
 * 構造体内部で関数宣言された
 */
class ERROR_INFO_3001 : public VMErrorParameter {
public :
	string funcName;
	ERROR_INFO_3001( string funcName ) : VMErrorParameter( ERROR_3001 ){
		this->funcName = funcName;
	}
};


/* 
 * 不明なシンボルタイプ
 */
class ERROR_INFO_4001 : public VMErrorParameter {
public :
	string symbolName;
	ESymbolType symbolType;
	ERROR_INFO_4001( string symbolName , ESymbolType symbolType ) : VMErrorParameter( ERROR_4001 ){
		this->symbolName = symbolName;
		this->symbolType = symbolType;
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