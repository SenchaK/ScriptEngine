#include "vmassemble_reader.h"
#include "vm_mnemonic_define.h"


namespace SenchaVM{
namespace Assembly{
static const char* Location( int location ){
	if( location == EMnemonic::MEM_S ) return "S";
	if( location == EMnemonic::MEM_L ) return "L";
	return "Unknown Location";
}


void VMTextFileWriter::execute(){
	int index = 0;
	AsmInfo* assemblyInfo = this->m_reader->getAssembly(0);
	while( assemblyInfo ){
		this->execAssemble( assemblyInfo );
		assemblyInfo = this->m_reader->getAssembly(++index);
	}
	fclose( this->m_fp );
}

void VMTextFileWriter::execAssemble( AsmInfo* assembly ){
	assert( assembly );

	this->m_pc = 0;
	fprintf( this->m_fp , "func %s\n" , assembly->name().c_str() );
	while( assembly->hasMore( this->m_pc ) ){
		fprintf( this->m_fp , "%08d:" , this->m_pc );
		int mnemonic = assembly->moveU8( this->m_pc );
		switch( mnemonic ){
#define __write_calc( name ) { fprintf( this->m_fp , name " " ); this->writeVarChain( assembly ); fprintf( this->m_fp , " , " ); this->writeVarChain( assembly ); fprintf( this->m_fp , "\n" ); }
		case EMnemonic::Mov    : __write_calc( "mov " ); break;
		case EMnemonic::Add    : __write_calc( "add " ); break;
		case EMnemonic::Sub    : __write_calc( "sub " ); break;
		case EMnemonic::Mul    : __write_calc( "mul " ); break;
		case EMnemonic::Div    : __write_calc( "div " ); break;
		case EMnemonic::Rem    : __write_calc( "rem " ); break;
		case EMnemonic::CmpEq  : __write_calc( "eq  " ); break;
		case EMnemonic::CmpNEq : __write_calc( "neq " ); break;
		case EMnemonic::CmpGeq : __write_calc( "geq " ); break;
		case EMnemonic::CmpG   : __write_calc( "g   " ); break;
		case EMnemonic::CmpLeq : __write_calc( "leq " ); break;
		case EMnemonic::CmpL   : __write_calc( "l   " ); break;
		case EMnemonic::LogAnd : __write_calc( "and " ); break;
		case EMnemonic::LogOr  : __write_calc( "or  " ); break;
#undef __write_calc
#define __write_jmp( name ) { fprintf( this->m_fp , name " [%08d]\n" , assembly->moveU32( this->m_pc ) ); }
		case EMnemonic::Jmp         : __write_jmp( "jmp " ); break;
		case EMnemonic::JumpZero    : __write_jmp( "jz  " ); break;
		case EMnemonic::JumpNotZero : __write_jmp( "jnz " ); break;
#undef __write_jmp
		case EMnemonic::ST :
			fprintf( this->m_fp , "st   %d\n" , assembly->moveU8( this->m_pc ) );
			break;
		case EMnemonic::Call :
			fprintf( this->m_fp , "call %d\n" , assembly->moveU32( this->m_pc ) );
			break;
		case EMnemonic::LD :
			fprintf( this->m_fp , "ld   %d\n" , assembly->moveU8( this->m_pc ) );
			break;
		case EMnemonic::Push :
			fprintf( this->m_fp , "push " );
			this->writeVarChain( assembly );
			fprintf( this->m_fp , "\n" );
			break;
		case EMnemonic::EndFunc :
			fprintf( this->m_fp , "end \n" );
			break;
		case EMnemonic::RET :
			fprintf( this->m_fp , "ret " );
			this->writeVarChain( assembly );
			fprintf( this->m_fp , "\n" );
			break;
		default :
			printf( "未登録のニーモニック ... %d\n" , mnemonic );
			break;
		}
	}
	fprintf( this->m_fp , "\n" );
}

void VMTextFileWriter::writeVarChain( AsmInfo* assembly ){
	int location = assembly->moveU8( this->m_pc );
	if( location == EMnemonic::LIT_VALUE ){
		fprintf( this->m_fp , "%.f" , assembly->moveDouble( this->m_pc ) );
		return;
	}
	if( location == EMnemonic::LIT_STRING ){
		fprintf( this->m_fp , "%s" , assembly->moveString( this->m_pc ).c_str() );
		return;
	}
	if( location == EMnemonic::REG ){
		fprintf( this->m_fp , "R%d" , assembly->moveU8( this->m_pc ) );
		return;
	}

	fprintf( this->m_fp , Location( location ) );
	size_t size = assembly->moveU32( this->m_pc );
	for( size_t i = 0 ; i < size ; i++ ){
		int isArray = assembly->moveU8( this->m_pc );
		if( isArray ){
			fprintf( this->m_fp , "[" ); 
		}

		int ifRef = assembly->moveU8( this->m_pc );
		if( ifRef ){
			fprintf( this->m_fp , "&" );
		}

		int addr = assembly->moveU32( this->m_pc );
		if( isArray ){
			fprintf( this->m_fp , "%d" , addr );
		}
		else{
			fprintf( this->m_fp , "[%d]" , addr );
		}
		if( isArray ){
			int sizeOf = assembly->moveU32( this->m_pc );
			int RIndex = assembly->moveU32( this->m_pc );
			fprintf( this->m_fp , "+(%d*R%d)]" ,  sizeOf , RIndex );
		}
	}
}


} // namespace Assembly
} // namespace SenchaVM