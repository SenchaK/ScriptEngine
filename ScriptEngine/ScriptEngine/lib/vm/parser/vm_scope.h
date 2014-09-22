#pragma once
#include "..\lexer\vm_lexer.h"
#include "..\symbol\vm_symbol.h"
#include "..\assembly\vm_assembly_info.h"
#include <stack>

namespace Sencha {
namespace VM{
namespace Assembly {

class Scope;
class Symtable;
class SymbolInfo;
typedef shared_ptr<Symtable> CSymtable;
typedef shared_ptr<Scope>    CScope;


class MethodInfo;
class Type;
class VMDriver;

// スコープ管理クラス
class Scope{
protected :
	enum EScopeType {
		Global         ,
		Function       , 
		Struct         , 
		StructMethod   ,
	};
	vector<Scope*> m_child      ; // 子スコープ 
	Scope* m_parent             ; // 親スコープ
	CSymtable m_symtable        ; // シンボルテーブル
	int m_scopeLevel            ; // スコープレベル
	EScopeType m_scopeType      ; // スコープ種類【関数・グローバル・構造体内部】
	string m_scopeName          ; // スコープ名
private :
	int getSearchSymbolType();
	SymbolInfo* const _addSymbol( string symbolName );
	Scope* getTopParent();	
public :
	// コンストラクタ
	// 名前とスコープレベルを初期値としてセットする
	Scope( string scopeName , int scopeLevel );

	// デストラクタ
	// 管理している子スコープを削除する。
	virtual ~Scope();

	// このスコープは構造体スコープであると通知
	void notifyStructMethodScope(){ m_scopeType = StructMethod; }

	// 構造体スコープかどうか
	bool isStructScope(){ return m_scopeType == Struct; }

	// 構造体メソッドスコープであるかどうか
	bool isStructMethodScope(){ return m_scopeType == StructMethod; }

	// スコープ名
	string ScopeName(){ return m_scopeName; }

	// スコープレベル
	int ScopeLevel(){ return m_scopeLevel; }

	// 一つ上の親スコープを取得
	Scope* const getParentScope(){ return m_parent; }

	// 一つ前のスコープに戻る
	Scope* const backToChildScope();

	// 子スコープへ進む
	// 進めた子スコープの参照を返す
	Scope* const goToChildScope( string name );

	// 構造体スコープへ進む
	// 進めた子スコープの参照を返す
	Type* const goToStructScope( string name );

	// 関数スコープへ進む
	// 進めた子スコープの参照を返す
	MethodInfo* const goToFunctionScope( string name );

	// 名前からスコープを探す
	Scope* const findScope( string scopeName );

	// シンボルを登録
	// 生成されたシンボルを返す
	SymbolInfo* const addSymbol( string symbolName );

	// シンボルを名前から検索して取得する
	// 対象の名前シンボルを返す
	SymbolInfo* const getSymbol( string name );

	// テーブルインデックスからシンボルを取得
	// 対象のシンボルを返す
	SymbolInfo* const getSymbol( int index );

	// スコープのフルネームを取得する。
	// 親スコープ名.子スコープ名と連結された文字列がフルネームとなる
	string toFullName( const string& funcName );

	// このスコープが管理するシンボルリストを返す
	const vector<SymbolInfo*>& getSymbols(){
		return m_symtable->getSymbols();
	}

	// 一番上のスコープから対象の名前のスコープを検索する。
	Scope* findScopeFromTop( string scopeName );

	// 現在のスコープが持つ子スコープの数を取得する
	// 孫スコープはカウントしない
	int getChildScopeCount(){ return m_child.size(); }

	// 対象のシンボルタイプ属性を持った、このスコープに存在するシンボルの数を取得する
	// 小階層、親階層に存在するシンボル数は含めない
	int getSymbolCount( int symbolMask );

	// 現在のスコープにあわせたシンボル数を取得する
	// グローバルスコープ:グローバル属性のシンボルのみ
	// 関数スコープ      :ローカル属性のシンボルのみ
	// 構造体スコープ    :構造体属性のシンボルのみ
	int getSymbolCountInScopeAttribute();

	// 現在のスコープから見つかる全ての子スコープの親階層を含めたシンボル総数を取得し、
	// 最も高いシンボル総数を返す
	int getSymbolCountMaxInAllScope( ESymbolType symbolType );

	// 親スコープの持つシンボル数を返す
	int getParentSymbolCount( ESymbolType symbolType );

	// 依存関係にある全ての親スコープを辿り、
	// 持っているシンボル総数を取得
	int getAllParentSymbolCount( ESymbolType symbolType );

	// 依存関係にある全ての親スコープの持つシンボル総数及び
	// 現在のスコープの所持するシンボル総数すべての合計値を取得する
	int getAllSymbolCount( ESymbolType symbolType );

	// 対象の名前のシンボルをこのスコープ、もしくは親階層が管理しているかどうか
	bool hasContainSymbol( string name );
};

// 構造体などの型情報の場合
class Type : public Scope {
public :
	Type( string scopeName , int scopeLevel ) : Scope( scopeName , scopeLevel ){
	}
	string Name(){
		return this->ScopeName();
	}
	int SizeOf();
};


// 関数スコープ
class MethodInfo : public Scope {
public :
	MethodInfo( string scopeName , int scopeLevel ) : Scope( scopeName , scopeLevel ){
	}
};

// 名前空間スコープ
class Package : public Scope {
private :
	VMBuiltIn* m_built_in;
public :
	Package( string scopeName , int scopeLevel );
	virtual ~Package();
	void insertMethod( string methodName , void(*function)(VMDriver*) );
};


} // namespace Assembly
} // namespace VM
} // namespace Sencha
