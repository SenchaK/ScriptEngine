#include "vmassembler.h"
#include "vmregister.h"


namespace SenchaVM{
namespace Assembly{



AsmInfo* VMDriver::currentAssembly(){
	return this->m_reader->getAssembly( m_funcAddr );
}

VMDriver::VMDriver( IAssembleReader* reader , VMBuiltIn* built_in ){
	this->m_funcAddr = 0;
	this->m_pc = 0;
	this->m_stacksize = 0;
	this->m_staticsize = 0;
	this->m_localAddr = 0;
	this->m_callStackIndex = 0;
	this->m_push = 0;
	memset( &m_callStack , 0 , sizeof( m_callStack ) );
	this->initialize( reader , built_in , 2048 , 1024 );
}

void VMDriver::initialize( IAssembleReader* reader , VMBuiltIn* built_in , size_t stacksize , size_t staticsize ){
	assert( stacksize > 0 );
	assert( staticsize > 0 );
	assert( reader );
	this->m_reader = reader;
	this->m_built_in = built_in;
	this->m_stacksize = stacksize;
	this->m_staticsize = staticsize;
	this->m_local = CMemory( new Memory[stacksize] , std::default_delete<Memory[]>() );
	this->m_static = CMemory( new Memory[staticsize] , std::default_delete<Memory[]>() );
	memset( &m_callStack , 0 , sizeof( m_callStack ) );
}

Memory& VMDriver::getLocal( int addres ){
	assert( addres >= 0 && addres < (int)m_stacksize );
	return m_local.get()[addres];
}
Memory& VMDriver::getStatic( int addres ){
	VM_ASSERT( addres >= 0 && addres < (int)m_staticsize );
	return m_static.get()[addres];
}
void VMDriver::setLocal( int addres , Memory& m ){
	VM_ASSERT( addres >= 0 && addres < (int)m_stacksize );
	Memory& src = m_local.get()[addres];
	src.setMemory( m );
}
void VMDriver::setStatic( int addres , Memory& m ){
	VM_ASSERT( addres >= 0 && addres < (int)m_staticsize );
	m_static.get()[addres].setMemory( m );
}
void VMDriver::setMemory( Memory& src , Memory& value ){
	src.setMemory( value );
}

Memory& VMDriver::getMemory( int location , int address ){
	static Memory Null;
	switch( location ){
	case EMnemonic::MEM_L :
		return this->getLocal( m_localAddr + address );
	case EMnemonic::MEM_S :
		return this->getStatic( address );
	}
	return Null;
}

unsigned char VMDriver::getByte( int funcAddr , int pc ){
	AsmInfo* assembly = this->m_reader->getAssembly( funcAddr );
	assert( assembly );
	return assembly->getCommand( pc );
}

bool VMDriver::isActive(){
	if( !this->currentAssembly() ) return false;
	if( this->currentAssembly()->hasMore( this->m_pc ) ) return true;
	if( this->m_funcAddr >= 0 ) return true;
	return false;
}

void VMDriver::execute(){
	assert( this->currentAssembly() );
	while( this->isActive() ){
		unsigned char content = this->getByte( m_funcAddr , m_pc );
		m_pc++;
		switch( content ){
			case EMnemonic::Mov :
				_mov();
				break;
			case EMnemonic::Add :
				_add();
				break;
			case EMnemonic::Sub :
				_sub();
				break;
			case EMnemonic::Mul :
				_mul();
				break;
			case EMnemonic::Div :
				_div();
				break;
			case EMnemonic::Rem :
				_rem();
				break;
			case EMnemonic::Inc :
				_inc();
				break;
			case EMnemonic::Dec :
				_dec();
				break;
			case EMnemonic::Push :
				_push();
				break;
			case EMnemonic::Pop :
				_pop();
				break;
			case EMnemonic::Call :
				_call();
				break;
			case EMnemonic::ST :
				_st();
				break;
			case EMnemonic::LD :
				_ld();
				break;
			case EMnemonic::EndFunc :
				_endFunc();
				break;

			case EMnemonic::CmpGeq : 
			case EMnemonic::CmpG :
			case EMnemonic::CmpLeq : 
			case EMnemonic::CmpL :
			case EMnemonic::CmpEq : 
			case EMnemonic::CmpNEq :
				_cmp( content );
				break;
			case EMnemonic::LogOr :
			case EMnemonic::LogAnd :
				_log( content );
				break;
			case EMnemonic::Jmp :
				_jmp();
				break;
			case EMnemonic::JumpZero :
				_jumpzero();
				break;
			case EMnemonic::JumpNotZero :
				_jumpnotzero();
				break;
			case EMnemonic::RET :
				_ret();
				break;
		}
	}
}

AsmInfo* VMDriver::findFunction( string name ){
	return this->m_reader->getAssembly( name );
}

int VMDriver::getFunctionAddr( string name ){
	AsmInfo* assembly = this->findFunction( name );
	if( assembly ){
		return assembly->addr();
	}
	printf( "not found function %s\n" , name.c_str() );
	return -1;
}

void VMDriver::getFunction( string func ){
	m_funcAddr = getFunctionAddr( func );
}

void VMDriver::vmsetup(){
	m_callStackIndex = 0;
	m_pc = 0;
}

Memory& VMDriver::createOrGetMemory(){
	static Memory literalMemory;

	int location = this->currentAssembly()->moveU8( this->m_pc );
	if( location == EMnemonic::LIT_VALUE ){
		literalMemory.setMemory( Memory( this->currentAssembly()->moveDouble( this->m_pc ) , "" ) );
		return literalMemory;
	}
	if( location == EMnemonic::LIT_STRING ){
		literalMemory.setMemory( Memory( 0 , this->currentAssembly()->moveString( this->m_pc ) ) );
		return literalMemory;
	}
	if( location == EMnemonic::REG ){
		return R_STACK::getMemory( this->currentAssembly()->moveU8( this->m_pc ) );
	}

	int address = 0;
	size_t size = this->currentAssembly()->moveU32( this->m_pc );
	bool isVariable = false;
	if( location == EMnemonic::MEM_S ) isVariable = true;
	if( location == EMnemonic::MEM_L ) isVariable = true;
	if( isVariable ){
		for( size_t i = 0 ; i < size ; i++ ){
			int isArray = this->currentAssembly()->moveU8( this->m_pc );
			int ifRef = this->currentAssembly()->moveU8( this->m_pc );
			if( ifRef ){
			}

			int addr = 0;
			addr += this->currentAssembly()->moveU32( this->m_pc );
			if( isArray ){
				int sizeOf = this->currentAssembly()->moveU32( this->m_pc );
				int RIndex = this->currentAssembly()->moveU32( this->m_pc );
				addr += sizeOf * ((int)R_STACK::getMemory( RIndex ).value);
			}
			address += addr;
		}
	}
	return this->getMemory( location , address );
}

void VMDriver::_mov(){
	Memory& src = this->createOrGetMemory();
	Memory& dest = this->createOrGetMemory();
	this->setMemory( src , dest );
}

void VMDriver::_add(){
	Memory& src = this->createOrGetMemory();
	Memory& dest = this->createOrGetMemory();
	this->setMemory( src , src + dest );
}

void VMDriver::_sub(){
	Memory& src = this->createOrGetMemory();
	Memory& dest = this->createOrGetMemory();
	this->setMemory( src , src - dest );
}

void VMDriver::_mul(){
	Memory& src = this->createOrGetMemory();
	Memory& dest = this->createOrGetMemory();
	this->setMemory( src , src * dest );
}

void VMDriver::_div(){
	Memory& src = this->createOrGetMemory();
	Memory& dest = this->createOrGetMemory();
	this->setMemory( src , src / dest );
}

void VMDriver::_rem(){
	Memory& src = this->createOrGetMemory();
	Memory& dest = this->createOrGetMemory();
	this->setMemory( src , src % dest );
}

void VMDriver::_inc(){
	Memory& src = this->createOrGetMemory();
	this->setMemory( src , ++src );
}

void VMDriver::_dec(){
	Memory& src = this->createOrGetMemory();
	this->setMemory( src , --src );
}


void VMDriver::_cmp( int cmpType ){
	bool result = 0;
	Memory& src = this->createOrGetMemory();
	Memory& dest = this->createOrGetMemory();
	switch( cmpType ){
		case EMnemonic::CmpGeq : result = src >= dest; break;
		case EMnemonic::CmpG   : result = src >  dest; break;
		case EMnemonic::CmpLeq : result = src <= dest; break;
		case EMnemonic::CmpL   : result = src <  dest; break;
		case EMnemonic::CmpEq  : result = src == dest; break;
		case EMnemonic::CmpNEq : result = src != dest; break;
	}
	setMemory( src , Memory( result , "" ) );
}

void VMDriver::_log( int logType ){
	bool result = 0;
	Memory& src = this->createOrGetMemory();
	Memory& dest = this->createOrGetMemory();
	switch( logType ){
		case EMnemonic::LogOr  : result = ( ( src != 0 ) || ( dest != 0 ) ); break;
		case EMnemonic::LogAnd : result = ( ( src != 0 ) && ( dest != 0 ) ); break;
	}
	setMemory( src , Memory( result , "" ) );
}

void VMDriver::_jmp(){
	int jmpaddr = currentAssembly()->moveU32( this->m_pc );
	this->m_pc = jmpaddr;
}

void VMDriver::_jumpzero(){
	int jmpaddr = currentAssembly()->moveU32( this->m_pc );
	Memory r0 = R_STACK::getMemory( 0 );
	if( r0 == 0 ){
		this->m_pc = jmpaddr;
	}
}

void VMDriver::_jumpnotzero(){
	int jmpaddr = currentAssembly()->moveU32( this->m_pc );
	Memory r0 = R_STACK::getMemory( 0 );
	if( r0 != 0 ){
		this->m_pc = jmpaddr;
	}
}

void VMDriver::_push(){
	size_t stackFrame = currentAssembly()->stackFrame();
	Memory& m = this->createOrGetMemory();
	Memory& set = getLocal( stackFrame + m_push + m_localAddr );
	m_push++;
	setMemory( set , m );
}

void VMDriver::_pop(){
	m_push--;
}

void VMDriver::_call(){
	assert( m_callStackIndex >= 0 && m_callStackIndex < STK_SIZE );
	struct funcinfoS{
		unsigned int address : 24;
		unsigned int type    :  8;
	};
	union {
		funcinfoS info;
		int int_value;
	} func;
	func.int_value = currentAssembly()->moveU32( this->m_pc );

	m_callStack[m_callStackIndex].funcAddr = m_funcAddr;
	m_callStack[m_callStackIndex].prog     = m_pc;
	m_callStackIndex++;
	this->m_localAddr += currentAssembly()->stackFrame();
	if( func.info.type == 1 ){
		assert( this->m_built_in );
		this->m_built_in->indexAt( func.info.address )->exec( this );
		this->_endFunc();
		return;
	}
	this->m_funcAddr = func.info.address;
	this->m_pc = 0;
}

void VMDriver::_st(){
	int RIndex = this->currentAssembly()->moveU8( this->m_pc );
}

void VMDriver::_ld(){
	int RIndex = this->currentAssembly()->moveU8( this->m_pc );
}

void VMDriver::_ret(){
	Memory& m = this->createOrGetMemory();
	R_STACK::setMemory( 0 , m );
}

void VMDriver::_endFunc(){
	m_callStackIndex--;
	if( m_callStackIndex < 0 ){
		m_funcAddr = -1;
		return;
	}
	assert( m_callStackIndex < STK_SIZE );
	this->m_funcAddr   = m_callStack[m_callStackIndex].funcAddr;
	this->m_pc         = m_callStack[m_callStackIndex].prog;
	this->m_localAddr -= currentAssembly()->stackFrame();
}


/* ****************************************************************************** *
 * public
 * ****************************************************************************** */ 
void VMDriver::executeFunction( string funcName ){
	this->getFunction( funcName );
	this->vmsetup();
	this->execute();
}

Memory& VMDriver::popMemory(){
	this->_pop();
	return this->getLocal( m_localAddr + m_push );
}



} // Assembly
} // SenchaVM