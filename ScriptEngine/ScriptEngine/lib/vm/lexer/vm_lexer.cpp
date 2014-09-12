#include "vm_lexer.h"
#include <assert.h>

namespace Sencha {
namespace VM {
using namespace std;

static Token EndToken( "" , Token::Type::END_TOKEN );
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
	int type;
};
syntax2WordS SYNTAX2[] = {
	{ '+' , '=' , Token::Type::AddAssign  } , 
	{ '-' , '=' , Token::Type::SubAssign  } , 
	{ '*' , '=' , Token::Type::MulAssign  } , 
	{ '/' , '=' , Token::Type::DivAssign  } , 
	{ '%' , '=' , Token::Type::RemAssign  } , 
	{ '=' , '=' , Token::Type::Equal      } , 
	{ '!' , '=' , Token::Type::NotEqual   } , 
	{ '>' , '=' , Token::Type::GEq        } , 
	{ '<' , '=' , Token::Type::LEq        } , 
	{ '+' , '+' , Token::Type::Inc        } , 
	{ '-' , '-' , Token::Type::Dec        } , 
	{ '|' , '|' , Token::Type::LogicalOr  } , 
	{ '&' , '&' , Token::Type::LogicalAnd } ,
};
//	------------------------------------------------------
//	1文字シンタックス
//	------------------------------------------------------
struct syntax1WordS {
	char c;
	int type;
};
syntax1WordS SYNTAX1[] = {
	{ '!' , Token::Type::Not			    } , // !
	{ ',' , Token::Type::Comma			    } , // ,
	{ ';' , Token::Type::Semicolon		    } , // ;
	{ ':' , Token::Type::Colon    		    } , // :
	{ '[' , Token::Type::Lbracket  		    } , // [
	{ ']' , Token::Type::Rbracket  		    } , // ]
	{ '(' , Token::Type::Lparen    		    } , // (
	{ ')' , Token::Type::Rparen    		    } , // )
	{ '{' , Token::Type::BeginChunk		    } , // {
	{ '}' , Token::Type::EndChunk		    } , // }
	{ '>' , Token::Type::Greater  		    } , // >
	{ '<' , Token::Type::Lesser  	    	} , // <
	{ '.' , Token::Type::Dot     	    	} , // .
	{ '=' , Token::Type::Assign    		    } , // =
	{ '+' , Token::Type::Add    		    } , // +
	{ '-' , Token::Type::Sub			    } , // -
	{ '*' , Token::Type::Mul    		    } , // *
	{ '/' , Token::Type::Div    		    } , // /
	{ '%' , Token::Type::Rem    		    } , // %
	{ '$' , Token::Type::VariableSymbol	    } , // $
};

static Token isSyntax2( char c1 , char c2 ){
	Token result( "" , Token::Type::NONCE );
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
static Token isSyntax1( char c ){
	Token result( "" , Token::Type::NONCE );
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



Lexer::Lexer( CStream stream ){
	assert( stream.get() ); 
	this->_initialize( stream );
}

Lexer::~Lexer(){
}

Token& Lexer::getToken() {
	return getToken(0);
}

Token& Lexer::getToken( int ofs ){
	size_t pos = m_tokenIndex + ofs;
	while( pos >= m_tokens.size() ){
		Token& result = _execute();
		if( result == Token::Type::END_TOKEN ){
			break;
		}
	}
	if( pos < m_tokens.size() ){
		return m_tokens[pos];
	}
	return EndToken;
}

// virtual
ITokenContainer* Lexer::next(){
	this->m_tokenIndex++;
	return &this->getToken();
}

// virtual
ITokenContainer* Lexer::back(){
	this->m_tokenIndex--;
	return &this->getToken();
}

// virtual
ITokenContainer* Lexer::current(){
	return &this->getToken();
}

// virtual
ITokenContainer* Lexer::offset( int ofs ){
	return &this->getToken( ofs );
}

// virtual
bool Lexer::hasNext(){
	if( !this->m_stream->hasNext() ){
		if( this->m_tokenIndex >= m_tokens.size() ){
			return false;
		}
	}
	return true;
}

Token& Lexer::peekToken(){
	assert( m_tokens.size() > 0 );
	return m_tokens[m_tokens.size()-1];
}

void Lexer::_initialize( CStream stream ){
	assert( stream.get() );
	m_stream = stream;
	m_textIndex = 0;
	m_tokenIndex = 0;
}

Token& Lexer::_execute(){	
	while( true ){
		_isS();
		if( _isComment() ){
			continue;
		}
		break;
	}
	if( _isSyntax2Wd() ){
		return this->peekToken();
	}
	if( _isSyntax1Wd() ){
		return this->peekToken();
	}
	if( _isLetter() ){
		return this->peekToken();
	}
	if( _isDigit() ){
		return this->peekToken();
	}
	if( _isString() ){
		return this->peekToken();
	}
	return EndToken;
}

bool Lexer::_isComment(){
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


void Lexer::_isS(){
	char c = _getc();
	while( c == '\t' || c == ' ' || c == '\n' || c == '\r' ){
		_advance();
		c = _getc();
	}
}
bool Lexer::_isSyntax2Wd(){
	char c1 = _getc(); _advance();
	char c2 = _getc(); _backstep();
	Token token = isSyntax2( c1 , c2 );
	if( token != Token::Type::NONCE ){
		_advance();
		_advance();
		m_tokens.push_back( token );
		return true;
	}
	return false;
}
bool Lexer::_isSyntax1Wd(){
	char c = _getc();
	Token token = isSyntax1( c );
	if( token != Token::Type::NONCE ){
		_advance();
		m_tokens.push_back( token );
		return true;
	}
	return false;
}

// 識別子解析
// 一文字目はかならず[A-Za-z_]でなければいけない
bool Lexer::_isLetter(){
	char c = _getc();
	if( !isLetter( c ) ){
		return false;
	}
	string text = "";
	int type = Token::Type::Letter;

	while( isLetter( c ) || isDigit( c ) ){	
		text += c;
		_advance();
		c = _getc();
	}

	if( text.compare( "function" ) == 0 )	{ type = Token::Type::Function	 ; }
	if( text.compare( "switch" ) == 0 )		{ type = Token::Type::Switch	 ; }
	if( text.compare( "for" ) == 0 )		{ type = Token::Type::For		 ; }
	if( text.compare( "while" ) == 0 )		{ type = Token::Type::While		 ; }
	if( text.compare( "if" ) == 0 )			{ type = Token::Type::If		 ; }
	if( text.compare( "else" ) == 0 )       { type = Token::Type::Else       ; }
	if( text.compare( "continue" ) == 0 )	{ type = Token::Type::Continue	 ; }
	if( text.compare( "break" ) == 0 )		{ type = Token::Type::Break		 ; }
	if( text.compare( "yield" ) == 0 )		{ type = Token::Type::YIELD		 ; }
	if( text.compare( "return" ) == 0 )		{ type = Token::Type::Return	 ; }
	if( text.compare( "struct" ) == 0 )		{ type = Token::Type::Struct  	 ; }
	if( text.compare( "namespace" ) == 0 )	{ type = Token::Type::Namespace	 ; }
	if( text.compare( "as" ) == 0 )	        { type = Token::Type::As         ; }
	if( text.compare( "array" ) == 0 )	    { type = Token::Type::Array      ; }

	m_tokens.push_back( Token( text , type ) );
	return true;
}

bool Lexer::_isDigit(){
	char c = _getc();
	if( !isDigit( c ) ){
		return false;
	}
	string text = "";
	int type = Token::Type::Digit;
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
	m_tokens.push_back( Token( text , type ) );
	return true;
}
bool Lexer::_isString(){
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
	m_tokens.push_back( Token( text , Token::Type::String ) );
	return true;
}

bool Lexer::_isEof(){
	char c = _getc();
	if( c == EOF ) return true;
	return false;
}

char Lexer::_getc(){
	size_t index = m_textIndex;
	while( index >= m_text.length() ){
		if( m_stream->hasNext() ){
			m_text += m_stream->getByte();
			continue;
		}
		break;
	}
	if( index < m_text.length() ){
		return m_text[index];
	}
	return EOF;
}

void Lexer::_advance(){
	m_textIndex++;
}

void Lexer::_backstep(){
	m_textIndex--;
}

} // namespace VM
} // namespace Sencha