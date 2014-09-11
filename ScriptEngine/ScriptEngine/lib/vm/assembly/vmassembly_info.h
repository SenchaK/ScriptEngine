#pragma once
#include "../../util/util_stream.h"
#include "../lexer/vmlexer.h"

namespace SenchaVM{
namespace Assembly{
typedef Sencha::Util::byte vmbyte;
class VMDriver;


class AsmInfo {
private :
	string m_name;
	size_t m_stackFrame;
	size_t m_addr;
	size_t m_args;
	vector<vmbyte> m_code;
public :
	void clearBytes(){
		m_code.clear();
	}
	void pushByte( vmbyte byteData ){
		m_code.push_back( byteData );
	}
	void setBytes( CStream stream ){
		while( stream->hasNext() ){
			m_code.push_back( stream->getByte() );
		}
	}
	void setArgs( size_t args ){
		m_args = args;
	}
	void setName( const string& name ){
		m_name = name;
	}
	void setAddress( size_t address ){
		m_addr = address;
	}
	void setStackFrame( size_t stackFrame ){
		m_stackFrame = stackFrame;
	}
	string& name(){
		return m_name;
	}
	size_t Args(){
		return m_args;
	}
	size_t stackFrame(){
		return m_stackFrame;
	}
	size_t addr(){
		return m_addr;
	}
	size_t CodeSize(){
		return m_code.size();
	}
	bool equal( string& name ){
		return this->m_name.compare( name ) == 0;
	}

	bool hasMore( int pc );
	vmbyte getCommand( const int& pc );
	unsigned __int8  moveU8( int& pc );
	unsigned __int16 moveU16( int& pc );
	unsigned __int32 moveU32( int& pc );
	double moveDouble( int& pc );
	string moveString( int& pc );
};



/*
 * 組み込み関数
 * パーザに登録する
 */
class VMBuiltInFunction {
private :
	string m_name;                        // マッピング関数名
	void (*function)( VMDriver* driver ); // コールバック関数
public :
	bool equal( string& src ){
		if( this->m_name.compare( src ) == 0 ){
			return true;
		}
		return false;
	}
	VMBuiltInFunction( string name , void (*built_in_function)( VMDriver* ) ){
		this->m_name = name;
		this->function = built_in_function;
	}
	void exec( VMDriver* driver ){
		if( this->function ){
			this->function( driver );
		}
	}
};

struct VMCallStack {
	int funcAddr;
	int prog;	
};

class VMBuiltIn {
private :
	vector<VMBuiltInFunction*> built_in_function;
public :
	void entryFunction( VMBuiltInFunction* func ){
		this->built_in_function.push_back( func );
	}

	void clear(){
		for( size_t i = 0 ; i < this->built_in_function.size() ; i++ ){
			delete this->built_in_function[i];
		}
		this->built_in_function.clear();
	}

	VMBuiltInFunction* indexAt( size_t index ){
		if( index >= this->built_in_function.size() ) return NULL;
		return this->built_in_function[index];
	}

	int find( string& funcName ){
		for( size_t i = 0 ; i < this->built_in_function.size() ; i++ ){
			if( this->built_in_function[i]->equal( funcName ) ){
				return i;
			}
		}
		return -1;
	}

	~VMBuiltIn(){
		this->clear();
	}
};

class VMAssembleCollection {
private :
	vector<AsmInfo*> Asm;
public :
	~VMAssembleCollection(){
		this->clear();
	}
	void entryAssembly( AsmInfo* code ){
		assert( code );
		code->setAddress( this->Asm.size() );
		this->Asm.push_back( code );
	}
	size_t count(){
		return this->Asm.size();
	}
	AsmInfo* const indexAt( size_t index ){
		if( index >= this->count() ) return NULL;
		return this->Asm[index];
	}
	int find( string& funcName ){
		for( size_t i = 0 ; i < this->count() ; i++ ){
			if( this->indexAt(i)->equal( funcName ) ){
				return i;
			}
		}
		return -1;
	}
private :
	void clear(){
		for( size_t i = 0 ; i < this->Asm.size() ; i++ ){
			delete this->Asm[i];
		}
		this->Asm.clear();
	}
};


} // namespace Assembly
} // namespace SenchaVM