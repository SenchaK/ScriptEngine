#pragma once
#include "..\lexer\vmlexer.h"
#include "..\symbol\vmsymbol.h"
#include "..\assembly\vmassembly_info.h"
#include <stack>

namespace SenchaVM {
namespace Assembly {

class Scope;
class Symtable;
class SymbolInfo;
typedef shared_ptr<Symtable> CSymtable;
typedef shared_ptr<Scope>    CScope;



// スコープ管理クラス
class Scope{
private :
	enum EScopeType {
		Global         ,
		Function       , 
		Struct         , 
		StructMethod   ,
	};

	vector<Scope*> m_child      ; // 子スコープ 
	Scope*         m_parent     ; // 親スコープ
	CSymtable      m_symtable   ; // シンボルテーブル
	int            m_scopeLevel ; // スコープレベル
	EScopeType     m_scopeType  ; // スコープ種類【関数・グローバル・構造体内部】
	string         m_scopeName  ; // スコープ名
private :
	int getSearchSymbolType();
	SymbolInfo* const _addSymbol( string symbolName );
	Scope* getTopParent();
	
public :
	void notifyStructMethodScope(){ m_scopeType = StructMethod; }
	bool isStructScope(){ return m_scopeType == Struct; }
	bool isStructMethodScope(){ return m_scopeType == StructMethod; }
	string ScopeName(){ return m_scopeName; }
	int ScopeLevel(){ return m_scopeLevel; }
	Scope* const getParentScope(){ return m_parent; }
	Scope* const backToChildScope();
	Scope* const goToChildScope( string name );
	Scope* const goToStructScope( string name );
	Scope* const goToFunctionScope( string name );
	Scope* const findScope( string scopeName );
	SymbolInfo* const addSymbol( string symbolName );
	SymbolInfo* const getSymbol( string name );
	SymbolInfo* const getSymbol( int index );

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

	const vector<SymbolInfo*>& getChildren();
	bool hasContainSymbol( string name );
	Scope( string scopeName , int scopeLevel );
	~Scope();
};


}
}
