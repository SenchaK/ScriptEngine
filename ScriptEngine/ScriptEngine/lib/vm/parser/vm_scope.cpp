#include "vm_parser.h"
#include "vm_scope.h"
#include "..\lexer\vm_lexer.h"
#include "..\vm_assembler.h"
#include "..\symbol\vm_symbol.h"



using namespace std;
#define PARSER_ASSERT VM_ASSERT


namespace Sencha {
namespace VM {
namespace Assembly {

// *****************************************************************************************
// class Scope
// *****************************************************************************************
Scope* Scope::getTopParent(){
	if( m_parent ){
		return m_parent->getTopParent();
	}
	return this;
}

Scope* const Scope::findScope( string scopeName ){
	for( unsigned int i = 0 ; i < m_child.size() ; i++ ){
		if( m_child[i]->m_scopeName == scopeName ){
			return m_child[i];
		}
	}
	for( unsigned int i = 0 ; i < m_child.size() ; i++ ){
		Scope* scope = m_child[i]->findScope( scopeName );
		if( scope ){
			return scope;
		}
	}
	return NULL;
}

Scope* Scope::findScopeFromTop( string scopeName ){
	Scope* parent = getTopParent();
	if( parent ){
		return parent->findScope( scopeName );
	}
	return NULL;
}

bool Scope::hasContainSymbol( string name ){
	if( m_parent ){
		if( m_parent->hasContainSymbol( name ) ){
			return true;
		}
	}
	SymbolInfo* symbol = m_symtable->findSymbol( name );
	if( symbol ){
		return true;
	}
	return false;
}

SymbolInfo* const Scope::getSymbol( string name ){
	SymbolInfo* result = NULL;
	if( m_parent ){
		result = m_parent->getSymbol( name );
		if( result ){
			return result;
		}
	}
	result = m_symtable->findSymbol( name );
	return result;
}

SymbolInfo* const Scope::getSymbol( int index ){
	return m_symtable->getSymbol( index );
}

/*
 * 構造体スコープである場合は型名.関数名の形式に変換する。
 */ 
string Scope::toFullName( const string& funcName ){
	if( this->isStructScope() ){
		return this->m_scopeName + "." + funcName;
	}
	return funcName;
}

Scope::Scope( string scopeName , int scopeLevel ){
	m_scopeName = scopeName;
	m_parent = NULL;
	m_symtable = CSymtable( new Symtable() );
	m_scopeLevel = scopeLevel;
	m_scopeType = Global;
}


Scope* const Scope::goToChildScope( string name ){
	Scope* newScope = new Scope( name , m_scopeLevel + 1 );
	newScope->m_parent = this;
	newScope->m_scopeType = this->m_scopeType;
	m_child.push_back( newScope );
	size_t pos = m_child.size() - 1;
	return m_child[pos];
}

Type* const Scope::goToStructScope( string name ){
	Type* newScope = new Type( name , m_scopeLevel + 1 );
	newScope->m_parent = this;
	newScope->m_scopeType = Struct;
	m_child.push_back( newScope );
	size_t pos = m_child.size() - 1;
	return newScope;
}

MethodInfo* const Scope::goToFunctionScope( string name ){
	MethodInfo* newScope = new MethodInfo( name , m_scopeLevel + 1 );
	newScope->m_parent = this;
	newScope->m_scopeType = Function;
	if( this->m_scopeType == Struct ){
		newScope->m_scopeType = StructMethod;
	}
	m_child.push_back( newScope );
	size_t pos = m_child.size() - 1;
	return newScope;
}

Scope* const Scope::backToChildScope(){
	return m_parent;
}

Scope::~Scope(){
	for( unsigned int i = 0 ; i < m_child.size() ; i++ ){
		Scope* child = m_child[i];
		delete child;
	}
	m_parent = NULL;
	m_scopeLevel = 0;
}

int Scope::getSearchSymbolType(){
	switch( m_scopeType ){
		case Global       : return VariableGlobal;
		case Function     : return VariableLocal ;
		case StructMethod : return VariableLocal ;
		case Struct       : return VariableField ;
	}
	return 0;
}

/* private */
SymbolInfo* const Scope::_addSymbol( string symbolName ){
	int topAddr = 0;
	switch( m_scopeType ){
	case Global   :
		topAddr = this->getAllSymbolCount( VariableGlobal );
		break;
	case Function :
	case StructMethod :
		topAddr = this->getAllSymbolCount( VariableLocal );
		break;
	case Struct :
		topAddr = this->getAllSymbolCount( VariableField );
		break;
	}
	m_symtable->entrySymbol( new SymbolInfo( symbolName , 1 , (ESymbolType)getSearchSymbolType() , false , this->m_scopeLevel ) );
	SymbolInfo* symbol = m_symtable->peekSymbol();

	symbol->Addr( topAddr );
	return symbol;
}
/* public */

// 対象のシンボルタイプ属性を持った、このスコープに存在するシンボルの数を取得する
// 小階層、親階層に存在するシンボル数は含めない
int Scope::getSymbolCount( int symbolMask ){
	return m_symtable->getSymbolCount( symbolMask );
}

// 現在のスコープにあわせたシンボル数を取得する
// グローバルスコープ:グローバル属性のシンボルのみ
// 関数スコープ      :ローカル属性のシンボルのみ
// 構造体スコープ    :構造体属性のシンボルのみ
int Scope::getSymbolCountInScopeAttribute(){
	return m_symtable->getSymbolCount( getSearchSymbolType() );
}

// 現在のスコープから見つかる全ての子スコープの親階層を含めたシンボル総数を取得し、
// 最も高いシンボル総数を返す
int Scope::getSymbolCountMaxInAllScope( ESymbolType symbolType ){
	int result = getAllSymbolCount( symbolType );
	int count  = 0;
	for( unsigned int i = 0 ; i < m_child.size() ; i++ ){
		count = m_child[i]->getSymbolCountMaxInAllScope( symbolType );
		if( count >= result ){
			result = count;
		}
	}
	return result;
}

// 親スコープの持つシンボル数を返す
int Scope::getParentSymbolCount( ESymbolType symbolType ){
	if( m_parent ){
		return m_parent->getSymbolCount( symbolType );
	}
	return 0;
}

// 依存関係にある全ての親スコープを辿り、
// 持っているシンボル総数を取得
int Scope::getAllParentSymbolCount( ESymbolType symbolType ){
	int result = 0;
	if( m_parent ){
		result += m_parent->getAllParentSymbolCount( symbolType );
	}
	result += getParentSymbolCount( symbolType );
	return result;
}

// 依存関係にある全ての親スコープの持つシンボル総数及び
// 現在のスコープの所持するシンボル総数すべての合計値を取得する
int Scope::getAllSymbolCount( ESymbolType symbolType ){
	return getAllParentSymbolCount( symbolType ) + getSymbolCount( symbolType );
}

/* public */
SymbolInfo* const Scope::addSymbol( string symbolName ){
	return _addSymbol( symbolName );
}


int Type::SizeOf(){
	return this->m_symtable->sizeOf();
}


// *****************************************************************************************
// class Package
// *****************************************************************************************
Package::Package( string scopeName , int scopeLevel ) : Scope( scopeName , scopeLevel ){
	this->m_built_in = new VMBuiltIn();
}

// virtual 
Package::~Package(){
	delete this->m_built_in;
}

void Package::insertMethod( string methodName , void(*function)(VMDriver*) ){
	this->m_built_in->entryFunction( new VMBuiltInFunction( methodName , function ) );
	this->goToFunctionScope( methodName );
}



} // namespace Assembly
} // namespace VM
} // namespace Sencha
