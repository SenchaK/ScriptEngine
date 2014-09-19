#pragma once
#include "../vm_define.h"
#include "../lexer/vm_lexer.h"
#include "../vm_assembler.h"


namespace Sencha{
namespace VM{
namespace Assembly{

class Type;
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
	int m_scopeLevel;
	bool m_isArray;
	ESymbolType m_symbolType;
	bool m_isReference;
	Type* m_type;
public :
	SymbolInfo( string name , size_t arrayLength , ESymbolType symbolType , bool isReference , int scopeLevel );
	~SymbolInfo();
	string DataTypeName();
	int toCode();
	int MemorySizeOf();
	int DataTypeSizeOf();

	string& Name(){
		return m_name;
	}
	size_t Addr(){
		return m_addr;
	}
	bool IsReference(){
		return m_isReference;
	}
	ESymbolType SymbolType(){
		return m_symbolType;
	}
	int ScopeLevel(){
		return m_scopeLevel;
	}
	int ArrayLength(){
		return m_arrayLength;
	}
	void Addr( size_t value ){
		m_addr = value;
	}
	void Location( ESymbolType type ){
		m_symbolType = type;
	}
	void ArrayLength( size_t length ){ 
		m_arrayLength = length;
	}
	void IsReference( bool isReference ){
		m_isReference = isReference;
	}	
	bool isArray(){
		return m_isArray;
	}
	void isArray( bool isArray ){
		m_isArray = isArray;
	}
	void setType( Type* t ){
		m_type = t;
	}
	Type* const getType(){
		return m_type;
	}
	bool isGlobal(){
		return m_scopeLevel == SCOPE_LEVEL_GLOBAL;
	}
};

class Symtable {
private :
	vector<SymbolInfo*> m_symbolList;
public  :
	const vector<SymbolInfo*>& getSymbols(){ return m_symbolList; }
	int getSymbolCount( int symbolMask );
	int sizeOf();
	SymbolInfo* const findSymbol( string symbolName );
	SymbolInfo* const findSymbol( int addr );
	SymbolInfo* const getSymbol( int index );
	SymbolInfo* const peekSymbol();
	void entrySymbol( SymbolInfo* symbol );
	~Symtable();
};
typedef shared_ptr<Symtable> CSymtable;



} // namespace Assembly
} // namespace VM
} // namespace Sencha