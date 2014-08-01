#pragma once
#include "../vmdefine.h"
#include "../lexer/vmlexer.h"
#include "../vmassembler.h"


namespace SenchaVM{
namespace Assembly{


class SymbolInfo {
	friend class Symtable;
private :
	static SymbolInfo g_null;
public  :
	static const SymbolInfo& Null();
private :
	string m_name;
	size_t m_addr;
	size_t m_arrayLength;
	int    m_scopeLevel;
	int    m_arrayIndexR;
	bool   m_isArray;
	ESymbolType m_symbolType;
	bool m_isReference;
	bool m_isStruct;
	unsigned short m_dataTypeId;
	vector<SymbolInfo*> m_child;
	SymbolInfo* m_parent;
	SymbolInfo* m_classSymbol;
	SymbolInfo( SymbolInfo* src , SymbolInfo* parent );
public :
	const vector<SymbolInfo*>& getChilds(){ return m_child; }
	SymbolInfo* const getChild( int index ){ return m_child[index]; }
	SymbolInfo( string name , size_t arrayLength , ESymbolType symbolType , bool isReference , bool isStruct , int scopeLevel );
	~SymbolInfo();
	string Name(){
		return m_name;
	}
	size_t Addr(){
		return m_addr;
	}
	size_t TopSymbolAddr();
	bool IsReference(){
		return m_isReference;
	}
	size_t DataSize();
	ESymbolType SymbolType(){
		return m_symbolType;
	}
	int ScopeLevel(){
		return m_scopeLevel;
	}
	int ChildSymbolCount(){
		return m_child.size();
	}
	int getSymbolCount();
	int ArrayLength(){ return m_arrayLength; }
	void Addr( size_t value ){
		m_addr = value;
	}
	void SymbolType( ESymbolType type ){
		m_symbolType = type;
	}
	void ArrayLength( size_t length ){ 
		m_arrayLength = length;
	}
	void IsReference( bool isReference ){
		m_isReference = isReference;
	}	
	string SymbolTypeString(){
		switch( m_symbolType ){
		case Func           : return "Func";
		case VariableLocal  : return "VariableLocal" ;
		case VariableGlobal : return "VariableGlobal";
		case Struct         : return "Struct";
		}  
		return "unknown";
	}

	bool isArray(){ return m_isArray; }
	void isArray( bool isArray ){ m_isArray = isArray; }
	void setArrayIndexR( int RNo ){ this->m_arrayIndexR = RNo; }
	int getArrayIndexR(){ return this->m_arrayIndexR; }
	void setClass( SymbolInfo* classsymbol ){ m_classSymbol = classsymbol; }
	SymbolInfo* const getClass(){ return m_classSymbol; }
	SymbolInfo* const getSymbol( string name );
	SymbolInfo* const addSymbol( string name );
	// このシンボルのスコープは静的な領域に存在しているのかどうか
	bool isGlobal(){ return m_scopeLevel == SCOPE_LEVEL_GLOBAL; }
	int toAssembleCode();
	int isReferenceMember();
	void copyAndAddChildrenOfSymbol( SymbolInfo* symbol );
	void copyAndAddChildrenOfSymbol( const vector<SymbolInfo*>& symbolList );
	void setupChildrenAddresToParentAddresOffset();
};

class Symtable {
private :
	vector<SymbolInfo*> m_symbolList;
public  :
	const vector<SymbolInfo*>& getSymbols(){ return m_symbolList; }
	int getSymbolCount( int symbolMask );
	int getSymbolTotal();
	SymbolInfo* const findSymbol( string symbolName );
	SymbolInfo* const findSymbol( int addr );
	SymbolInfo* const getSymbol( int index );
	SymbolInfo* const peekSymbol();
	void entrySymbol( SymbolInfo* symbol );
	~Symtable();
};
typedef shared_ptr<Symtable> CSymtable;



} // namespace Assembly
} // namespace SenchaVM