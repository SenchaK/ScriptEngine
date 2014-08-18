#pragma once
#include "../../util/util_stream.h"
#include "../lexer/vmlexer.h"

namespace SenchaVM{
namespace Assembly{
typedef Sencha::Util::byte vmbyte;


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
	string& name(){
		return m_name;
	}
	void setStackFrame( size_t stackFrame ){
		m_stackFrame = stackFrame;
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


struct VMCallStack {
	int funcAddr;
	int prog;	
};

class VMAssembleCollection {
private :
	vector<AsmInfo*> Asm;
public :
	~VMAssembleCollection(){
		this->clear();
	}
	void entryAssembly( AsmInfo* code ){
		Asm.push_back( code );
	}
	size_t count(){
		return Asm.size();
	}
	AsmInfo* const indexAt( int index ){
		assert( index >= 0 && index < (int)Asm.size() );
		return Asm[index];
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
		for( size_t i = 0 ; i < Asm.size() ; i++ ){
			delete Asm[i];
		}
		Asm.clear();
	}
};


} // namespace Assembly
} // namespace SenchaVM