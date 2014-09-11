#pragma once

#pragma warning( disable : 4996 )
#include<string>
#include<vector>
#include<cassert>
#include<list>
#include<iostream>
#include<sstream>
#include<memory>


#define SCOPE_LEVEL_GLOBAL ( 0 )


namespace SenchaVM{

// トークンコンテナ
// トークナイザーに管理されるトークン要素はこれを継承する必要がある
class ITokenContainer {
};


// トークンインターフェース
// 構文解析を行うときはこのインターフェースを参照する
// 内部でどのように実装されるかは隠蔽するように作る
class ITokenizer {
public :
	// 一つ前にポジションを戻す
	// @return 一つ前のトークン
	virtual ITokenContainer* back() = 0;

	// トークンを一つ進める
	// @return 次のトークン
	virtual ITokenContainer* next() = 0;

	// 現在のトークンを返す
	// @return 現在のトークン
	virtual ITokenContainer* current() = 0;

	// 現在位置からのオフセット値を加算した位置のトークンを返す
	// @return 指定位置のトークン
	virtual ITokenContainer* offset( int ofs ) = 0;

	// 次のトークンが存在するのか
	// @return
	// ・これ以上すすめられない場合false
	// ・進められる場合はtrue
	virtual bool hasNext() = 0;
};

namespace Assembly{
class AsmInfo;

// シンボル種類
enum ESymbolType {
	Func           = 0x01 , 
	VariableField  = 0x02 ,
	VariableLocal  = 0x04 , 
	VariableGlobal = 0x08 ,
	Struct         = 0x10 ,
};

// アセンブル情報提供インターフェース
// 内部コードを取得する機能を提供する
class IAssembleReader {
public :
	// インデックス指定でアセンブリを取得
	virtual AsmInfo* getAssembly( int index ) = 0;
	// 名前指定でアセンブリを取得
	virtual AsmInfo* getAssembly( std::string index ) = 0;
};


} // namespace Assembly
} // namespace SenchaVM