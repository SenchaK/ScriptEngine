#define SWITCH_DEBUG
#ifdef SWITCH_DEBUG
	#define SWITCH_LOG VM_PRINT
	#define SWITCH_ASSERT VM_ASSERT
#else
	#define SWITCH_LOG(...)
	#define SWITCH_ASSERT(...)
#endif


#include "vmparser.h"
#include "..\lexer\vmlexer.h"
#include "..\vmassembler.h"
#include "..\symbol\vmsymbol.h"


namespace SenchaVM {
namespace Assembly {
// **************************************************************************
// switch�����
// **************************************************************************
//void Parser::_parse_switch( Context* param ){
//	SWITCH_LOG( "**switch��\n" );
//	SWITCH_ASSERT( getToken().type == TokenType::Switch ); nextToken();
//	SWITCH_ASSERT( getToken().type == TokenType::Lparen ); nextToken();
//}

} // namespace Assembly
} // namespace SenchaVM
