#include "vmlexer.h"
#include "../assembly/vm_mnemonic_define.h"
#include <assert.h>

#define LEXER_ASSERT assert



namespace SenchaVM {
using namespace std;
using namespace Assembly;

const string LETTER = 
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"_";
const string DIGIT = 
	"0123456789";


//	------------------------------------------------------
//	2文字シンタックス
//	------------------------------------------------------
struct syntax2WordS {
	char c1;
	char c2;
	TOKEN_TYPE type;
};
syntax2WordS SYNTAX2[] = {
	{ '+' , '=' , TokenType::AddAssign  } , 
	{ '-' , '=' , TokenType::SubAssign  } , 
	{ '*' , '=' , TokenType::MulAssign  } , 
	{ '/' , '=' , TokenType::DivAssign  } , 
	{ '%' , '=' , TokenType::RemAssign  } , 
	{ '=' , '=' , TokenType::Equal      } , 
	{ '!' , '=' , TokenType::NotEqual   } , 
	{ '>' , '=' , TokenType::GEq        } , 
	{ '<' , '=' , TokenType::LEq        } , 
	{ '+' , '+' , TokenType::Inc        } , 
	{ '-' , '-' , TokenType::Dec        } , 
	{ '|' , '|' , TokenType::LogicalOr  } , 
	{ '&' , '&' , TokenType::LogicalAnd } ,
};
//	------------------------------------------------------
//	1文字シンタックス
//	------------------------------------------------------
struct syntax1WordS {
	char c;
	TOKEN_TYPE type;
};
syntax1WordS SYNTAX1[] = {
	{ '!' , TokenType::Not			} , // !
	{ ',' , TokenType::Comma		} , // ,
	{ ';' , TokenType::Semicolon	} , // ;
	{ ':' , TokenType::Colon    	} , // :
	{ '[' , TokenType::Lbracket  	} , // [
	{ ']' , TokenType::Rbracket  	} , // ]
	{ '(' , TokenType::Lparen    	} , // (
	{ ')' , TokenType::Rparen    	} , // )
	{ '{' , TokenType::BeginChunk	} , // {
	{ '}' , TokenType::EndChunk	    } , // }
	{ '>' , TokenType::Greater  	} , // >
	{ '<' , TokenType::Lesser  	    } , // <
	{ '.' , TokenType::Dot     	    } , // .
	{ '=' , TokenType::Assign    	} , // =
	{ '+' , TokenType::Add    		} , // +
	{ '-' , TokenType::Sub			} , // -
	{ '*' , TokenType::Mul    		} , // *
	{ '/' , TokenType::Div    		} , // /
	{ '%' , TokenType::Rem    		} , // %
	{ '$' , TokenType::VariableSymbol	} , // $
};

static TOKEN isSyntax2( char c1 , char c2 ){
	TOKEN result( "" , TokenType::NONCE );
	for( int i = 0 ; i < sizeof(SYNTAX2)/sizeof(*SYNTAX2) ; i++ ){
		if( SYNTAX2[i].c1 == c1 && SYNTAX2[i].c2 == c2 ){
			result.type = SYNTAX2[i].type;
			result.text += SYNTAX2[i].c1;
			result.text += SYNTAX2[i].c2;
			return result;
		}
	}
	return result;
}
static TOKEN isSyntax1( char c ){
	TOKEN result( "" , TokenType::NONCE );
	for( int i = 0 ; i < sizeof(SYNTAX1)/sizeof(*SYNTAX1) ; i++ ){
		if( SYNTAX1[i].c == c ){
			result.type = SYNTAX1[i].type;
			result.text = SYNTAX1[i].c;
			return result;
		}
	}
	return result;
}
static bool isLetter( char c ){
	for( size_t i = 0 ; i < LETTER.length() ; i++ ){
		if( c == LETTER[i] ) return true;
	}
	return false;
}
static bool isDigit( char c ){
	for( size_t i = 0 ; i < DIGIT.length() ; i++ ){
		if( c == DIGIT[i] ) return true;
	}
	return false;
}
//	------------------------------------------------------

int TOKEN::toAssembleCode(){
	switch( type ){
		case TokenType::Add    : return EMnemonic::Add;
		case TokenType::Sub    : return EMnemonic::Sub;
		case TokenType::Mul    : return EMnemonic::Mul;
		case TokenType::Div    : return EMnemonic::Div;
		case TokenType::Rem    : return EMnemonic::Rem;
		case TokenType::Assign : return EMnemonic::Mov;
	}
	return 0;
}


LexcialReader::LexcialReader( CStream stream ){
	VM_ASSERT( stream.get() ); 
	this->_initialize( stream );
	this->_execute();
}
LexcialReader::~LexcialReader(){
	VM_PRINT( "LexcialReader::Finish\n" );
}
std::vector<TOKEN> LexcialReader::getResult(){
	return m_tokens;
}
void LexcialReader::_initialize( CStream stream ){
	VM_ASSERT( stream.get() );
	m_index = 0;
	while( stream->hasNext() ){
		m_text += (char)stream->getByte();
	}
}
void LexcialReader::_execute(){
	LEXER_ASSERT( m_text.length() > 0 );
	LEXER_ASSERT( m_index == 0 );

	while( !_isEof() ){
		_isS();
		if( _isComment() ){
			continue;
		}
		_isSyntax2Wd();
		if( _isLetter() ){
			continue;
		}
		if( _isDigit() ){
			continue;
		}
		if( _isString() ){
			continue;
		}
		_advance();
	}
}

bool LexcialReader::_isComment(){
	char c1 = _getc(); _advance();
	char c2 = _getc(); _backstep();
	if( c1 == '/' && c2 == '*' ){
		_advance(); // '/'
		_advance(); // '*'
		while( !_isEof() ){
			c1 = _getc(); _advance();
			c2 = _getc(); _backstep();
			if( c1 == '*' && c2 == '/' ){
				_advance(); // '*'
				_advance(); // '/'
				break;
			}
			_advance();
		}
		return true;
	}
	if( c1 == '/' && c2 == '/' ){
		_advance(); // '/'
		_advance(); // '/'
		while( !_isEof() ){
			c1 = _getc();
			if( c1 == '\n' ){
				_advance();
				break;
			}
			_advance();
		}
		return true;
	}
	return false;
}


void LexcialReader::_isS(){
	char c = _getc();
	while( c == '\t' || c == ' ' || c == '\n' || c == '\r' ){
		_advance();
		c = _getc();
	}
}
void LexcialReader::_isSyntax2Wd(){
	char c1 = _getc(); _advance();
	char c2 = _getc(); _backstep();
	TOKEN token = isSyntax2( c1 , c2 );
	if( token.type != TokenType::NONCE ){
		_advance();
		m_tokens.push_back( token );
		return;
	}
	_isSyntax1Wd();
}
void LexcialReader::_isSyntax1Wd(){
	char c = _getc();
	TOKEN token = isSyntax1( c );
	if( token.type != TokenType::NONCE ){
		m_tokens.push_back( token );
	}
}

// 識別子解析
// 一文字目はかならず[A-Za-z_]でなければいけない
bool LexcialReader::_isLetter(){
	char c = _getc();
	if( !isLetter( c ) ){
		return false;
	}
	string text = "";
	TOKEN_TYPE tokenType = TokenType::Letter;

	while( isLetter( c ) || isDigit( c ) ){	
		text += c;
		_advance();
		c = _getc();
	}

	if( text.compare( "function" ) == 0 )	{ tokenType = TokenType::Function	; }
	if( text.compare( "switch" ) == 0 )		{ tokenType = TokenType::Switch	    ; }
	if( text.compare( "for" ) == 0 )		{ tokenType = TokenType::For		; }
	if( text.compare( "while" ) == 0 )		{ tokenType = TokenType::While		; }
	if( text.compare( "if" ) == 0 )			{ tokenType = TokenType::If		    ; }
	if( text.compare( "else" ) == 0 )       { tokenType = TokenType::Else       ; }
	if( text.compare( "continue" ) == 0 )	{ tokenType = TokenType::Continue	; }
	if( text.compare( "break" ) == 0 )		{ tokenType = TokenType::Break		; }
	if( text.compare( "yield" ) == 0 )		{ tokenType = TokenType::YIELD		; }
	if( text.compare( "return" ) == 0 )		{ tokenType = TokenType::Return	    ; }
	if( text.compare( "struct" ) == 0 )		{ tokenType = TokenType::Struct  	; }
	if( text.compare( "namespace" ) == 0 )	{ tokenType = TokenType::Namespace	; }
	if( text.compare( "string" ) == 0 )		{ tokenType = TokenType::AsString   ; }
	if( text.compare( "int" ) == 0 )	    { tokenType = TokenType::AsInteger  ; }
	if( text.compare( "as" ) == 0 )	        { tokenType = TokenType::As         ; }
	if( text.compare( "array" ) == 0 )	    { tokenType = TokenType::Array      ; }

	m_tokens.push_back( TOKEN( text , tokenType ) );
	return true;
}

bool LexcialReader::_isDigit(){
	char c = _getc();
	if( !isDigit( c ) ){
		return false;
	}
	string text = "";
	TOKEN_TYPE tokenType = TokenType::Digit;
	while( isDigit( c ) ){
		text += c;
		_advance();
		c = _getc();
	}
	if( c == '.' ){
		text += c;
		_advance();
		c = _getc();
	}
	while( isDigit( c ) ){
		text += c;
		_advance();
		c = _getc();
	}
	m_tokens.push_back( TOKEN( text , tokenType ) );
	return true;
}
bool LexcialReader::_isString(){
	char c = _getc();
	if( c != '"' ) return false;
	_advance();
	c = _getc();
	string text = "";
	while( c != '"' ){
		text += c;
		_advance();
		c = _getc();
	}
	_advance();
	m_tokens.push_back( TOKEN( text , TokenType::String ) );
	return true;
}
bool LexcialReader::_isEof(){
	char c = _getc();
	if( c == EOF ) return true;
	return false;
}
char LexcialReader::_getc(){
	if( m_index >= m_text.length() ) return EOF;
	return m_text[m_index];
}
void LexcialReader::_advance(){
	m_index++;
}
void LexcialReader::_backstep(){
	m_index--;
}


}
//	namespace SenchaVM