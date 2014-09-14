#pragma once
#include "..\vm_define.h"
#include "..\..\util\binary.h"
#include "..\..\util\stream.h"


namespace Sencha {
namespace VM {
using namespace std;
using namespace Sencha;
using namespace Util;




//	----------------------------------------------------------
//	éöãÂèÓïÒ
//	----------------------------------------------------------
class Token : public ITokenContainer {
public :
	class Type {
	public :
		enum {
			NONCE           , // 
			Letter          , // [A-Za-z_][A-Za-z_0-9]*
			Dot             , // .
			Assign          , // =
			Add		        , // +
			Sub		        , // -
			Mul		        , // *
			Div		        , // /
			Rem  	        , // %
			RefSymbol = Rem ,
			AddAssign       , // +=
			SubAssign       , // -=
			MulAssign       , // *=
			DivAssign       , // /=
			RemAssign       , // %=
			Equal           , // ==
			NotEqual        , // !=
			GEq             , // >=
			Greater         , // >
			LEq             , // <=
			Lesser          , // <
			LogicalOr       , // ||
			LogicalAnd      , // &&
			Inc             , // ++
			Dec             , // --
			Digit           , // [0-9][0-9]*.?[0-9]*f?
			DoubleQt        , // "
			String          , // "string_value"
			Not             , // !
			Comma           , // ,
			Semicolon       , // ;
			Colon           , // :
			Lbracket        , // [
			Rbracket        , // ]
			Lparen          , // (
			Rparen          , // )
			BeginChunk      , // {
			EndChunk        , // }
			Function        , // "function"
			VariableSymbol  , // "$Å`"
			Switch          , // "switch"
			For             , // "for"
			While           , // "while"
			If              , // "if"
			Else            , // "else"
			Continue        , // "continue"
			Break           , // "break"
			YIELD           , // "yield"
			Return          , // "return"
			Struct          , // "struct"
			Class           , // "class"
			As              , // as
			Array           , // array
			Namespace       , // "namespace"
			END_TOKEN       ,
		};
	};
	std::string text;
	int type;
	Token( std::string txt , int typ ){
		this->text = txt;
		this->type = typ;
	}
	bool operator == ( int type ){
		if( this->type == type ) return true;
		return false;
	}
	bool operator != ( int type ){
		if( this->type != type ) return true;
		return false;
	}
	int toAssembleCode();
};


//	----------------------------------------------------------
//	éöãÂèÓïÒ
//	----------------------------------------------------------
class Lexer : public ITokenizer {
private :
	CStream m_stream;
	std::string m_text;
	size_t m_textIndex;
	std::vector<Token> m_tokens;
	size_t m_tokenIndex;
public  :
	Lexer( CStream stream );
	virtual ~Lexer();
	virtual ITokenContainer* back();
	virtual ITokenContainer* next();
	virtual ITokenContainer* current();
	virtual ITokenContainer* offset( int ofs );
	virtual bool hasNext();
private :
	Token& getToken();
	Token& getToken( int ofs );
	void seek( int pos ){ m_tokenIndex = pos; }
	Token& peekToken();
	Token& _execute();
	void _initialize( CStream stream );
	void _advance();
	void _backstep();
	char _getc();
	bool _isEof();
	void _isS();
	bool _isComment();
	bool _isSyntax2Wd();
	bool _isSyntax1Wd();
	bool _isLetter();
	bool _isDigit();
	bool _isString();
};
typedef std::shared_ptr<Lexer> CLexer;

} // namespace VM
} // namespace Sencha
