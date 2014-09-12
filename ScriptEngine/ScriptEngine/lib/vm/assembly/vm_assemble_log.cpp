#include "vm_assemble_log.h"
#include "vm_mnemonic_define.h"


namespace Sencha{
namespace VM {
namespace Assembly{
static const char* Location( int location ){
	if( location == EMnemonic::MEM_S ) return "S";
	if( location == EMnemonic::MEM_L ) return "L";
	return "Unknown Location";
}


void VMAssembleLog::execute(){
	int index = 0;
	AsmInfo* assemblyInfo = this->m_reader->getAssembly(0);
	while( assemblyInfo ){
		this->execAssemble( assemblyInfo );
		assemblyInfo = this->m_reader->getAssembly(++index);
	}
}

void VMAssembleLog::execAssemble( AsmInfo* assembly ){
	assert( assembly );
	
	this->m_pc = 0;
	this->print( "func %s\n" , assembly->name().c_str() );
	while( assembly->hasMore( this->m_pc ) ){
		this->print( "%08d:" , this->m_pc );
		int mnemonic = assembly->moveU8( this->m_pc );
		switch( mnemonic ){
		case EMnemonic::Mov         : this->writeCalc( "mov" , assembly ); break;
		case EMnemonic::Add         : this->writeCalc( "add" , assembly ); break;
		case EMnemonic::Sub         : this->writeCalc( "sub" , assembly ); break;
		case EMnemonic::Mul         : this->writeCalc( "mul" , assembly ); break;
		case EMnemonic::Div         : this->writeCalc( "div" , assembly ); break;
		case EMnemonic::Rem         : this->writeCalc( "rem" , assembly ); break;
		case EMnemonic::CmpEq       : this->writeCalc( "eq"  , assembly ); break;
		case EMnemonic::CmpNEq      : this->writeCalc( "neq" , assembly ); break;
		case EMnemonic::CmpGeq      : this->writeCalc( "geq" , assembly ); break;
		case EMnemonic::CmpG        : this->writeCalc( "g"   , assembly ); break;
		case EMnemonic::CmpLeq      : this->writeCalc( "leq" , assembly ); break;
		case EMnemonic::CmpL        : this->writeCalc( "l"   , assembly ); break;
		case EMnemonic::LogAnd      : this->writeCalc( "and" , assembly ); break;
		case EMnemonic::LogOr       : this->writeCalc( "or"  , assembly ); break;
		case EMnemonic::Jmp         : this->writeJmp ( "jmp" , assembly ); break;
		case EMnemonic::JumpZero    : this->writeJmp ( "jz"  , assembly ); break;
		case EMnemonic::JumpNotZero : this->writeJmp ( "jnz" , assembly ); break;
		case EMnemonic::ST          : this->st       ( assembly );         break;
		case EMnemonic::Call        : this->call     ( assembly );         break;
		case EMnemonic::LD          : this->ld       ( assembly );         break;
		case EMnemonic::Push        : this->push     ( assembly );         break;
		case EMnemonic::EndFunc     : this->end      ( assembly );         break;
		case EMnemonic::RET         : this->ret      ( assembly );         break;
		default :
			printf( "未登録のニーモニック ... %d\n" , mnemonic );
			break;
		}
	}
	this->print( "\n" );
}

void VMAssembleLog::writeCalc( const char* name , AsmInfo* assembly ){
	this->print( "%5s " , name );
	this->writeVarChain( assembly );
	this->print( " , " );
	this->writeVarChain( assembly );
	this->print( "\n" );
}

void VMAssembleLog::writeJmp( const char* name , AsmInfo* assembly ){
	this->print( "%5s [%08d]\n" , name , assembly->moveU32( this->m_pc ) );
}

void VMAssembleLog::writeVarChain( AsmInfo* assembly ){
	int location = assembly->moveU8( this->m_pc );
	if( location == EMnemonic::LIT_VALUE ){
		this->print( "%.f" , assembly->moveDouble( this->m_pc ) );
		return;
	}
	if( location == EMnemonic::LIT_STRING ){
		this->print( "%s" , assembly->moveString( this->m_pc ).c_str() );
		return;
	}
	if( location == EMnemonic::REG ){
		this->print( "R%d" , assembly->moveU8( this->m_pc ) );
		return;
	}

	this->print( Location( location ) );
	size_t size = assembly->moveU32( this->m_pc );
	for( size_t i = 0 ; i < size ; i++ ){
		int isArray = assembly->moveU8( this->m_pc );
		if( isArray ){
			this->print( "[" ); 
		}

		int ifRef = assembly->moveU8( this->m_pc );
		if( ifRef ){
			this->print( "&" );
		}

		int addr = assembly->moveU32( this->m_pc );
		if( isArray ){
			this->print( "%d" , addr );
		}
		else{
			this->print( "[%d]" , addr );
		}
		if( isArray ){
			int sizeOf = assembly->moveU32( this->m_pc );
			int RIndex = assembly->moveU32( this->m_pc );
			this->print( "+(%d*R%d)]" ,  sizeOf , RIndex );
		}
	}
}


void VMAssembleLog::call( AsmInfo* assembly ){
	struct funcinfoS{
		unsigned int address : 24;
		unsigned int type    :  8;
	};
	union {
		funcinfoS info;
		int int_value;
	} func;
	func.int_value = assembly->moveU32( this->m_pc );

	this->print( "%5s %d" , "call" , func.info.address );
	if( func.info.type == 1 ){
		this->print( "(built in function)" );
	}
	this->print( "\n" );
}

void VMAssembleLog::st( AsmInfo* assembly ){
	this->print( "%5s %d\n" , "st" , assembly->moveU8( this->m_pc ) );
}

void VMAssembleLog::ld( AsmInfo* assembly ){
	this->print( "%5s %d\n" , "ld" , assembly->moveU8( this->m_pc ) );
}

void VMAssembleLog::push( AsmInfo* assembly ){
	this->print( "%5s " , "push" );
	this->writeVarChain( assembly );
	this->print( "\n" );
}

void VMAssembleLog::end( AsmInfo* assembly ){
	this->print( "%5s\n" , "end" );
}

void VMAssembleLog::ret( AsmInfo* assembly ){
	this->print( "%5s " , "ret" );
	this->writeVarChain( assembly );
	this->print( "\n" );
}



} // namespace Assembly
} // VM
} // namespace Sencha