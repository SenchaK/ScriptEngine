#include "vmassembler.h"
#include "vmregister.h"

#define MNEMONIC_MASTER_DEBUG (0)
#define MNEMONIC_UNIT_DEBUG   (1)
#define MNEMONIC_DEBUG        (0)

#ifdef MNEMONIC_DEBUG
	#if MNEMONIC_DEBUG == MNEMONIC_MASTER_DEBUG
		#define MNEMONIC_LOG    VM_PRINT
		#define MNEMONIC_ASSERT VM_ASSERT
	#elif EXPRESSION_DEBUG == EXPRESSION_UNIT_DEBUG
		#define MNEMONIC_LOG    printf
		#define MNEMONIC_ASSERT assert
	#endif
#else
	#define MNEMONIC_LOG(...)
	#define MNEMONIC_ASSERT(...)
#endif
	
	
namespace SenchaVM{
namespace Assembly{
#define MEMORY_MASTER_DEBUG (0)
#define MEMORY_UNIT_DEBUG   (1)
#define MEMORY_DEBUG        (0)

#ifdef MEMORY_DEBUG
	#if MEMORY_DEBUG == MEMORY_MASTER_DEBUG
		#define MEMORY_LOG    VM_PRINT
		#define MEMORY_ASSERT VM_ASSERT
	#elif MEMORY_DEBUG == MEMORY_UNIT_DEBUG
		#define MEMORY_LOG    printf
		#define MEMORY_ASSERT assert
	#endif
#else
	#define MEMORY_LOG(...)
	#define MEMORY_ASSERT(...)
#endif



static string MemoryInfoString( memoryinfoS& info , int stackp ){
	ostringstream stream;
	switch( info.type ){
	case EMnemonic::LIT_VALUE : 
		stream << info.value;
		break;
	case EMnemonic::LIT_STRING :
		stream << "\"" << info.value_string << "\"";
		break;
	case EMnemonic::MEM_L : 
		stream << info.label << ":";
		stream << "L" << info.index + stackp; 
		if( info.isArray )          { stream << "[]";                     }
		if( info.isReferenceMember ){ stream << "+" << info.offsetAddres; }
		if( info.isReference )      { stream << "*"; }
		break;
	case EMnemonic::MEM_S : 
		stream << info.label << ":";
		stream << "S" << info.index;
		if( info.isArray )          { stream << "[]";                     }
		if( info.isReferenceMember ){ stream << "+" << info.offsetAddres; }
		if( info.isReference )      { stream << "*"; }
		break;
	case EMnemonic::MEM_THIS_P :
		stream << info.label << ":";
		stream << info.index + stackp;
		if( info.isArray )          { stream << "[]";                     }
		if( info.isReferenceMember ){ stream << "+" << info.offsetAddres; }
		if( info.isReference )      { stream << "*"; }
		break;
	case EMnemonic::REG :
		stream << "R" << info.index;
		break;
	}
	return stream.str();
}

void valuepairS::dump( int mnemonic , int stackp ){
	MEMORY_LOG( "%s:" , EMnemonic::toString( mnemonic ).c_str() );
	MEMORY_LOG( "%s" , MemoryInfoString( src , stackp ).c_str() );
	MEMORY_LOG( "," );
	MEMORY_LOG( "%s" , MemoryInfoString( dest , stackp ).c_str() );
	MEMORY_LOG( "\n" );
}


// ***********************************************************************
// class VMDriverInterface
// ***********************************************************************
Memory& VMDriverInterface::popMemory(){
	VM_ASSERT( m_driver );
	return m_driver->popMemory();
}
VMDriverInterface::VMDriverInterface(){
}
void VMDriverInterface::onInit( VMDriver* driver ){
	VM_ASSERT( driver );
	m_driver = driver;
}
void VMDriverInterface::Return( const double& value ){
	SenchaVM::Assembly::R_STACK::setReturnMemory( SenchaVM::Assembly::Memory( value , "" ) );
}
void VMDriverInterface::Return( const string& value ){
	SenchaVM::Assembly::R_STACK::setReturnMemory( SenchaVM::Assembly::Memory( 0 , value ) );
}
void VMDriverInterface::Return( const double& value , const string& string_value ){
	SenchaVM::Assembly::R_STACK::setReturnMemory( SenchaVM::Assembly::Memory( value , string_value ) );
}
void VMDriverInterface::VMSleep( int sleeptime ){
	m_driver->VMSleep( sleeptime );
}



// ***********************************************************************
// VMSystemCallService
// ***********************************************************************
void VMSystemCallService::addSystemCallService( VMSystemCallService* service ){
	assert( service );
	service->onInit( m_driver );
	m_system.push_back( service );
}

/* virtual */
void VMSystemCallService::onInit( VMDriver* driver ){
	VMDriverInterface::onInit( driver );
	m_driver = driver;
}

/* virtual */
void VMSystemCallService::callFunction( string funcName ){
	for( list<VMSystemCallService*>::iterator iter = m_system.begin() ; iter != m_system.end() ; iter++ ){
		VMSystemCallService* service = *iter;
		service->callFunction( funcName );
	}
}
VMSystemCallService::~VMSystemCallService(){
	for( list<VMSystemCallService*>::iterator iter = m_system.begin() ; iter != m_system.end() ; iter++ ){
		VMSystemCallService* service = *iter;
		delete service;
	}
	m_system.clear();
}




// ***********************************************************************
// class VMDriver
// ***********************************************************************
VMDriver::VMDriver(){
	m_systemCall = CVMSystemCallService( new VMSystemCallService() );
	m_state = DRIVER_IDLE;
	m_sleepcount = 0;
	m_sleeptime = 0;
}

Memory& VMDriver::popMemory(){
	return getMemory( memoryinfoS( EMnemonic::MEM_L , --m_systemcall_push ) );
}

void VMDriver::setSystemCallService( CVMSystemCallService systemcall ){
	VM_ASSERT( systemcall.get() );
	m_systemCall = systemcall;
	m_systemCall->onInit( this );
}

void VMDriver::VMSleep( int sleeptime ){
	m_sleeptime = sleeptime;
	m_state = DRIVER_SLEEP;
}




static memoryinfoS CreateMemoryInfo( AsmInfo* assembly , int& pc ){
	memoryinfoS result;
	result.type = assembly->moveU8 ( pc );
	result.index = 0;
	result.offsetAddres = 0;
	result.value = 0;
	result.value_string = "";

	switch( result.type ){
	case EMnemonic::LIT_VALUE :
		result.value = assembly->moveDouble( pc );
		break;
	case EMnemonic::LIT_STRING :
		result.value_string = assembly->moveString( pc );
		break;
	case EMnemonic::MEM_L :
	case EMnemonic::MEM_S :
	case EMnemonic::MEM_THIS_P :
		result.isArray = assembly->moveU8 ( pc );
		result.isReferenceMember = assembly->moveU8( pc );
		result.isReference = assembly->moveU8( pc );
		result.index = assembly->moveU32( pc );
		if( result.isReferenceMember ){
			result.offsetAddres = assembly->moveU32( pc );
		}
		result.label = assembly->moveString( pc );
		break;
	case EMnemonic::REG :
		result.index = assembly->moveU32( pc );
		break;
	}
	return result;
}

// ***********************************************************************
// class VMMainRoutineDriver
// ***********************************************************************
static valuepairS CreateValuePair( AsmInfo* assembly , int& pc ){
	valuepairS result;
	result.src  = CreateMemoryInfo( assembly , pc ); // SRC
	result.dest = CreateMemoryInfo( assembly , pc ); // DEST
	VM_ASSERT( result.src.type != EMnemonic::LIT_VALUE );
	VM_ASSERT( result.src.type != EMnemonic::LIT_STRING );
	return result;
}
static valuepairS CreateValueSingle( AsmInfo* assembly , int& pc ){
	valuepairS result;
	result.dest = CreateMemoryInfo( assembly , pc );
	return result;
}
static Memory CreateMemory( SenchaVM::Assembly::AsmInfo* assembly , int& pc ){
	vmbyte type = assembly->moveU8( pc );
	Memory result;
	switch( type ){
	case EMnemonic::LIT_STRING :
		result.setMemory( Memory( 0 , assembly->moveString( pc ) ) );
		break;
	case EMnemonic::LIT_VALUE :
		result.setMemory( Memory( assembly->moveDouble( pc ) , "" ) );
		break;
	}
	return result;
}




valuepairS VMMainRoutineDriver::createValuePair(){
	return CreateValuePair( currentAssembly() , m_prog );
}

AsmInfo* VMMainRoutineDriver::currentAssembly(){
	return &m_AsmInfo[m_funcAddr];
}

VMMainRoutineDriver::VMMainRoutineDriver() : VMDriver(){
	m_funcAddr = 0;
	m_prog = 0;
	m_stacksize = 0;
	m_staticsize = 0;
	m_coroutinesize = 0;
	m_localAddr = 0;
	m_push = 0;
	m_systemcall_push = 0;
	m_basePointer = 0;
	m_callStackIndex = 0;
	memset( &m_callStack , 0 , sizeof( m_callStack ) );
}

void VMMainRoutineDriver::initialize( const VMAssembleCollection& context , size_t stacksize , size_t staticsize , size_t coroutinesize ){
	VM_ASSERT( stacksize > 0 );
	VM_ASSERT( staticsize > 0 );
	VM_ASSERT( coroutinesize > 0 );
	m_funcAddr = 0;
	m_prog = 0;
	m_stacksize = stacksize;
	m_staticsize = staticsize;
	m_coroutinesize = coroutinesize;
	m_localAddr = 0;
	m_push = 0;
	m_basePointer = 0;
	m_systemcall_push = 0;
	m_callStackIndex = 0;
	m_local = CMemory( new Memory[stacksize] , std::default_delete<Memory[]>() );
	m_static = CMemory( new Memory[staticsize] , std::default_delete<Memory[]>() );
	m_coroutine = CVMCoroutineDriver( new VMCoroutineDriver[coroutinesize] , std::default_delete<VMCoroutineDriver[]>() );
	for( unsigned int i = 0 ; i < coroutinesize ; i++ ){
		m_coroutine.get()->initialize( this , 2048 );
	}
//	m_AsmInfo = context.Asm;
	memset( &m_callStack , 0 , sizeof( m_callStack ) );
}

Memory& VMMainRoutineDriver::getLocal( int addres ){
	VM_ASSERT( addres >= 0 && addres < (int)m_stacksize );
	return m_local.get()[addres];
}
Memory& VMMainRoutineDriver::getStatic( int addres ){
	VM_ASSERT( addres >= 0 && addres < (int)m_staticsize );
	return m_static.get()[addres];
}
void VMMainRoutineDriver::setLocal( int addres , Memory& m ){
	VM_ASSERT( addres >= 0 && addres < (int)m_stacksize );
	Memory& src = m_local.get()[addres];
	src.setMemory( m );
}
void VMMainRoutineDriver::setStatic( int addres , Memory& m ){
	VM_ASSERT( addres >= 0 && addres < (int)m_staticsize );
	m_static.get()[addres].setMemory( m );
}
/* override */
void VMMainRoutineDriver::setMemory( Memory& src , Memory& value ){
	src.setMemory( value );
}

/* virtual */
Memory& VMMainRoutineDriver::getMemoryAbsolute( int memoryScope , int addres ){
	static Memory Null;
	if( memoryScope == EMnemonic::MEM_L ){
		return getLocal ( addres );
	}
	if( memoryScope == EMnemonic::MEM_S ){
		return getStatic( addres );
	}
	cout << "unknown memory type:" << EMnemonic::toString( memoryScope ) << " , " << memoryScope << endl;
	assert( 0 && "Not Found Memory" );
	return Null;
}

/* override */
Memory& VMMainRoutineDriver::getMemory( const memoryinfoS& info ){
	static Memory Null;
	VM_ASSERT( 
		info.type == EMnemonic::MEM_L ||
		info.type == EMnemonic::MEM_S ||
		info.type == EMnemonic::MEM_THIS_P || 
		info.type == EMnemonic::REG   );

	int offset = info.offsetAddres;
	int addres = info.index;
	if( info.isArray ){
		if( !info.isReferenceMember ){
			addres += R_STACK::getIndex();
		}
	}

	switch( info.type ){
	case EMnemonic::MEM_L :
		{
			addres += m_localAddr + m_basePointer;
			if( info.isReferenceMember ){
				if( info.isArray ){
					Memory& reference = getLocal( addres );
					return getMemoryAbsolute( reference.MemoryScope() , reference.PointerAddres() + offset + R_STACK::getIndex() );
				}
				Memory& reference = getLocal( addres );
				return getMemoryAbsolute( reference.MemoryScope() , reference.PointerAddres() + offset );
			}
			Memory& result = getLocal( addres );
			return result;
		}
		break;
	case EMnemonic::MEM_S :
		{
			if( info.isReferenceMember ){
				if( info.isArray ){
					Memory& reference = getLocal( addres );
					return getMemoryAbsolute( reference.MemoryScope() , reference.PointerAddres() + offset + R_STACK::getIndex() );
				}
				Memory& reference = getStatic( addres + info.offsetAddres );
				return getMemoryAbsolute( reference.MemoryScope() , reference.PointerAddres() + offset );
			}
			Memory& result = getStatic( addres );
			return result;
		}
		break;
	case EMnemonic::REG   :
		{
			Memory& result = R_STACK::getMemory( addres );
			return result;
		}
		break;
	}
	return Null;
}
/* override */
vmbyte VMMainRoutineDriver::getByte( int funcAddr , int pc ){
	VM_ASSERT( funcAddr >= 0 && funcAddr < (int)m_AsmInfo.size() );
	return m_AsmInfo[funcAddr].getCommand( pc );
}
/* override */
bool VMMainRoutineDriver::isProgramEnd(){
	if( m_state == DRIVER_END ){
		return true;
	}
	if( m_state == DRIVER_IDLE ){
		return true;
	}
	if( m_state == DRIVER_SLEEP ){
		if( m_sleepcount != m_sleeptime ){
			m_sleepcount++;
			return true;
		}
		m_sleepcount = 0;
		m_sleeptime = 0;
		m_state = DRIVER_EXECUTE;
	}
	if( m_funcAddr < 0 ){
		return true;
	}
	if( m_AsmInfo.size() <= 0 ){
		return true;
	}
	if( currentAssembly()->hasMore( m_prog ) ){
		return false;
	}
	return true;
}

void VMMainRoutineDriver::execute(){
	while( !isProgramEnd() ){
		vmbyte content = getByte( m_funcAddr , m_prog );
		m_prog++;

		switch( content ){
			case EMnemonic::PMov :
				_pmov();
				break;
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
			case EMnemonic::AsString :
				_asString();
				break;
			case EMnemonic::ArrayIndexSet :
				_arrayIndexSet();
				break;
			case EMnemonic::ArrayIndexAdd :
				_arrayIndexAdd();
				break;
			case EMnemonic::RET :
				_ret();
				break;
		}
	}
}

AsmInfo* VMMainRoutineDriver::findFunction( string name ){
	for( size_t i = 0 ; i < m_AsmInfo.size() ; i++ ){
		if( m_AsmInfo[i].name().compare( name ) == 0 ) return &m_AsmInfo[i];
	}
	return NULL;
}

int VMMainRoutineDriver::getFunctionAddr( string name ){
	for( size_t i = 0 ; i < m_AsmInfo.size() ; i++ ){
		if( m_AsmInfo[i].name().compare( name ) == 0 ) return i;
	}
	return -1;
}

void VMMainRoutineDriver::getFunction( string func ){
	m_funcAddr = getFunctionAddr( func );
}

void VMMainRoutineDriver::vmsetup(){
	m_state = DRIVER_EXECUTE;
	m_callStackIndex = 0;
	m_prog = 0;
	m_sleepcount = 0;
	m_sleeptime = 0;
}

void VMMainRoutineDriver::executeFunction( string funcName ){
	if( m_state == DRIVER_SLEEP ){
		execute();
		return;
	}
	getFunction( funcName );
	vmsetup();
	execute();
}

void VMMainRoutineDriver::_pmov(){
	valuepairS pair = createValuePair();
	pair.dump( EMnemonic::PMov , m_localAddr );
	Memory& src = getMemory( pair.src );
	if( pair.isMemory() ){
		int index = pair.dest.index;
		int memory_scope = pair.dest.type;
		bool referenceMemory = false;
		// レジスタに格納された参照値を渡す
		if( pair.dest.type == EMnemonic::REG ){
			referenceMemory = true;
		}
		// 参照型
		if( pair.dest.isReference == 1 ){
			referenceMemory = true;
		}
		// ローカル値型
		if( pair.dest.type == EMnemonic::MEM_L && pair.dest.isReference == 0 ){
			index += m_localAddr;
		}
		// 参照型の場合はメモリに格納されたアドレスと記憶領域指定
		if( referenceMemory ){
			Memory& dest = getMemory( pair.dest );
			index = dest.PointerAddres();
			memory_scope = dest.MemoryScope();
		}
		if( pair.dest.isArray > 0 ){
			index += R_STACK::getIndex();
		}
		src.setPointer( index , memory_scope );
	}
}
void VMMainRoutineDriver::_mov(){
	valuepairS pair = createValuePair();
	pair.dump( EMnemonic::Mov , m_localAddr + m_basePointer );

	Memory& src = getMemory( pair.src );
	if( pair.isMemory() ){
		Memory& value = getMemory( pair.dest );
		setMemory( src , value ); 
		return;
	}
	setMemory( src , Memory( pair.dest.value , pair.dest.value_string ) );
}
void VMMainRoutineDriver::_add(){
	valuepairS pair = createValuePair();
	pair.dump( EMnemonic::Add , m_localAddr );

	Memory& src = getMemory( pair.src );
	if( pair.isMemory() ){
		Memory& value = getMemory( pair.dest );
		setMemory( src , Memory( src.Value() + value.Value() , src.ValueString() + value.ValueString() ) ); 
		return;
	}
	setMemory( src , Memory( src.Value() + pair.dest.value , src.ValueString() +  pair.dest.value_string ) );
}

void VMMainRoutineDriver::_sub(){
	valuepairS pair = createValuePair();
	pair.dump( EMnemonic::Sub , m_localAddr );

	Memory& src = getMemory( pair.src );
	if( pair.isMemory() ){
		Memory& value = getMemory( pair.dest );
		setMemory( src , Memory( src.Value() - value.Value() , value.ValueString() ) ); 
		return;
	}
	setMemory( src , Memory( src.Value() - pair.dest.value , pair.dest.value_string ) );
}

void VMMainRoutineDriver::_mul(){
	valuepairS pair = createValuePair();
	pair.dump( EMnemonic::Mul , m_localAddr );

	Memory& src = getMemory( pair.src );
	if( pair.isMemory() ){
		Memory& value = getMemory( pair.dest );
		setMemory( src , Memory( src.Value() * value.Value() , value.ValueString() ) ); 
		return;
	}
	setMemory( src , Memory( src.Value() * pair.dest.value , pair.dest.value_string ) );
}

void VMMainRoutineDriver::_div(){
	valuepairS pair = createValuePair();
	pair.dump( EMnemonic::Div , m_localAddr );

	Memory& src = getMemory( pair.src );
	if( pair.isMemory() ){
		Memory& value = getMemory( pair.dest );
		setMemory( src , Memory( src.Value() / value.Value() , value.ValueString() ) ); 
		return;
	}
	setMemory( src , Memory( src.Value() / pair.dest.value , pair.dest.value_string ) );
}

void VMMainRoutineDriver::_rem(){
	valuepairS pair = createValuePair();
	pair.dump( EMnemonic::Rem , m_localAddr );

	Memory& src = getMemory( pair.src );
	if( pair.isMemory() ){
		Memory& value = getMemory( pair.dest );
		setMemory( src , Memory( (int)src.Value() % (int)value.Value() , value.ValueString() ) ); 
		return;
	}
	setMemory( src , Memory( (int)src.Value() % (int)pair.dest.value , pair.dest.value_string ) );
}

void VMMainRoutineDriver::_inc(){
	valuepairS pair = CreateValueSingle( currentAssembly() , m_prog );
	Memory& src = getMemory( pair.dest );
	if( pair.isMemory() ){
		setMemory( src , Memory( src.Value() + 1 , src.ValueString() )  );
	}
}

void VMMainRoutineDriver::_dec(){
	valuepairS pair = CreateValueSingle( currentAssembly() , m_prog );
	Memory& src = getMemory( pair.dest );
	if( pair.isMemory() ){
		setMemory( src , Memory( src.Value() - 1 , src.ValueString() )  );
	}
}


void VMMainRoutineDriver::_cmp( int cmpType ){
	valuepairS pair = createValuePair();
	pair.dump( cmpType , m_localAddr );
	Memory& src = getMemory( pair.src );
	double destValue = pair.dest.value;
	bool result = false;
	if( pair.isMemory() ){
		Memory& value = getMemory( pair.dest );
		destValue = value.Value();
	}

	switch( cmpType ){
		case EMnemonic::CmpGeq : result = src.Value() >= destValue; break;
		case EMnemonic::CmpG   : result = src.Value() >  destValue; break;
		case EMnemonic::CmpLeq : result = src.Value() <= destValue; break;
		case EMnemonic::CmpL   : result = src.Value() <  destValue; break;
		case EMnemonic::CmpEq  : result = ((long long)src.Value()) == ((long long)destValue); break;
		case EMnemonic::CmpNEq : result = ((long long)src.Value()) != ((long long)destValue); break;
	}
	setMemory( src , Memory( result , pair.dest.value_string ) );
}

void VMMainRoutineDriver::_log( int logType ){
	valuepairS pair = createValuePair();
	pair.dump( logType , m_localAddr );
	Memory& src = getMemory( pair.src );
	double destValue = pair.dest.value;
	bool result = false;
	if( pair.isMemory() ){
		Memory& value = getMemory( pair.dest );
		destValue = value.Value();
	}

	switch( logType ){
	case EMnemonic::LogOr  : result = ((long long)src.Value()) != 0 || ((long long)destValue) != 0; break;
	case EMnemonic::LogAnd : result = ((long long)src.Value()) != 0 && ((long long)destValue) != 0; break;
	}
	
	setMemory( src , Memory( result , pair.dest.value_string ) );
}

void VMMainRoutineDriver::_jmp(){
//#define JMP_PRINT(...)
#define JMP_PRINT MNEMONIC_LOG
	JMP_PRINT( "jmp!!=>" );
	int jmpaddr = currentAssembly()->moveU32( m_prog );
	JMP_PRINT( "%d" , jmpaddr );
	m_prog = jmpaddr;
	JMP_PRINT( "\n" );
#undef JMP_PRINT
}

void VMMainRoutineDriver::_jumpzero(){
//#define JMP_PRINT(...)
#define JMP_PRINT MNEMONIC_LOG
	JMP_PRINT( "jumpZero!!=>" );
	int jmpaddr = currentAssembly()->moveU32( m_prog );
	Memory r0 = getMemory( memoryinfoS( EMnemonic::REG , 0 ) );
	if( r0.Value() == 0 ){
		JMP_PRINT( "%d" , jmpaddr );
		m_prog = jmpaddr;
	}
	JMP_PRINT( "\n" );
#undef JMP_PRINT
}

void VMMainRoutineDriver::_jumpnotzero(){
//#define JMP_PRINT(...)
#define JMP_PRINT MNEMONIC_LOG
	JMP_PRINT( "jumpNotZero!!=>" );
	int jmpaddr = currentAssembly()->moveU32( m_prog );
	Memory r0 = getMemory( memoryinfoS( EMnemonic::REG , 0 ) );
	if( r0.Value() != 0 ){
		JMP_PRINT( "%d" , jmpaddr );
		m_prog = jmpaddr;
	}
	JMP_PRINT( "\n" );
#undef JMP_PRINT
}

void VMMainRoutineDriver::_asString(){
	valuepairS pair = CreateValueSingle( currentAssembly() , m_prog );
	Memory& value = getMemory( pair.dest );

	if( pair.isMemory() ){
		static char buf[256];
		sprintf_s<256>( buf , "%.f" , value.Value() );
		string string_value = buf;
		setMemory( value , Memory( value.Value() , string_value )  );
	}
}

void VMMainRoutineDriver::_push(){
	size_t stackFrame = currentAssembly()->stackFrame();
	valuepairS pair = CreateValueSingle( currentAssembly() , m_prog );
	int isReference = currentAssembly()->moveU8( m_prog );

	MEMORY_LOG( "Push %d [L%d]\n" , m_push + 1 , stackFrame + m_push + m_localAddr );
	Memory& set = getLocal( stackFrame + m_push + m_localAddr );
	m_push++;
	if( pair.isMemory() ){
		if( isReference ){
			Memory& dest_R = getMemory( pair.dest );
			int index = dest_R.PointerAddres();
			int memory_scope = dest_R.MemoryScope();
			set.setPointer( index , memory_scope );
			return;
		}
		Memory& value = getMemory( pair.dest );
		setMemory( set , value );
		return;
	}
	setMemory( set , Memory( pair.dest.value , pair.dest.value_string ) );
}

void VMMainRoutineDriver::_pop(){
	m_push--;
	m_basePointer--;
	MEMORY_LOG( "Pop %d\n" , m_push );
}

void VMMainRoutineDriver::_call(){
	VM_ASSERT( m_callStackIndex >= 0 && m_callStackIndex < STK_SIZE );
	m_localAddr += currentAssembly()->stackFrame();
	Memory memory = CreateMemory( currentAssembly() , m_prog );
	int nextFuncAddr = getFunctionAddr( memory.ValueString() );

	m_callStack[m_callStackIndex].funcAddr = m_funcAddr;
	m_callStack[m_callStackIndex].prog     = m_prog;
	m_callStackIndex++;
	m_basePointer = m_push;

	MEMORY_LOG( "call %s , push %d , stackFrame %d , localAddr %d\n" , memory.ValueString().c_str() , m_push , currentAssembly()->stackFrame() , m_localAddr );
	R_STACK::pushCalc();
	if( nextFuncAddr < 0 ){
		int prevBaseP = m_basePointer;
		m_systemcall_push = m_push;
		m_basePointer = 0;
		m_systemCall->callFunction( memory.ValueString() );
		_endFunc();
		int delta = m_push - m_systemcall_push;
		m_push = m_systemcall_push;
		m_basePointer = prevBaseP - delta;
		//cout << "BASE_P:" << m_basePointer << endl; 
		return;
	}
	m_funcAddr = nextFuncAddr;
	m_prog = 0;
}

void VMMainRoutineDriver::_ret(){
	valuepairS pair = CreateValueSingle( currentAssembly() , m_prog );
	VM_ASSERT( pair.isMemory() );
	Memory& dest = getMemory( pair.dest );
	R_STACK::setReturnMemory( dest );
	MEMORY_LOG( "Return [%d,%s]\n" , (int)dest.Value() , dest.ValueString().c_str() );
}

void VMMainRoutineDriver::_arrayIndexSet(){
	valuepairS pair = CreateValueSingle( currentAssembly() , m_prog );
	pair.dump( EMnemonic::ArrayIndexSet , m_localAddr );
	VM_ASSERT( pair.isMemory() );
	Memory& dest = getMemory( pair.dest );
	R_STACK::setIndex( (int)dest.Value() );
}

void VMMainRoutineDriver::_arrayIndexAdd(){
	valuepairS pair = CreateValueSingle( currentAssembly() , m_prog );
	pair.dump( EMnemonic::ArrayIndexAdd , m_localAddr );
	VM_ASSERT( pair.isMemory() );
	Memory& dest = getMemory( pair.dest );

	int index = R_STACK::getIndex();
	R_STACK::setIndex( (int)dest.Value() + index );
}

void VMMainRoutineDriver::_endFunc(){
	m_callStackIndex--;
	if( m_callStackIndex < 0 ){
		m_funcAddr = -1;
		m_state = DRIVER_END;
		MNEMONIC_LOG( "SCRIPT_END\n" );
		return;
	}
	R_STACK::popCalc();
	VM_ASSERT( m_callStackIndex < STK_SIZE );
	m_funcAddr = m_callStack[m_callStackIndex].funcAddr;
	m_prog     = m_callStack[m_callStackIndex].prog;
	m_localAddr -= currentAssembly()->stackFrame();
	MNEMONIC_LOG( "関数終了：一つ前に戻る %s \n" , currentAssembly()->name().c_str() );
}



// ***********************************************************************
// class VMCoroutineDriver
// ***********************************************************************
void VMCoroutineDriver::initialize( VMDriver* parentRoutine , size_t stacksize ){
	VM_ASSERT( parentRoutine );
	m_Parent = parentRoutine;
	m_local = CMemory( new Memory[stacksize] , std::default_delete<Memory[]>() );
}
/* override */
void VMCoroutineDriver::setMemory( Memory& src , Memory& value ){
}
/* override */
Memory& VMCoroutineDriver::getMemory( const memoryinfoS& info ){
	static Memory Null;
	VM_ASSERT( 
		info.type == EMnemonic::MEM_L ||
		info.type == EMnemonic::MEM_S ||
		info.type == EMnemonic::REG );
	int addres = info.index;
	switch( info.type ){
		case EMnemonic::MEM_L : return m_local.get()[addres];
		case EMnemonic::MEM_S : return m_Parent->getMemory( info );
		case EMnemonic::REG   : return R_STACK::getMemory( addres );
	}
	return Null;
}
/* override */
vmbyte VMCoroutineDriver::getByte( int funcAddr , int pc ){
	return m_Parent->getByte( funcAddr , pc );
}
/* override */
bool VMCoroutineDriver::isProgramEnd(){
	return m_Parent->isProgramEnd();
}

} // Assembly
} // SenchaVM