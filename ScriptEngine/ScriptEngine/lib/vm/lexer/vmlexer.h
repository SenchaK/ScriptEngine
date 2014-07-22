#pragma once
#include "..\vmdefine.h"
#include "..\..\util\util_binary.h"
#include "..\..\util\util_stream.h"


namespace Sencha {
	namespace Util {
		class Stream;
		class TextReader;
		typedef shared_ptr<Stream> CStream;
		typedef shared_ptr<TextReader> CTextReader;
	}
}


namespace SenchaVM {
using namespace std;
using namespace Sencha;
using namespace Util;

class TokenType {
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
		VariableSymbol  , // "$`"
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

		As              , // as
		Array           , // array
		AsString        , // string( $xxx )
		AsInteger       , // int( $xxx )

		Namespace       , // "namespace"
		END_TOKEN       ,
	};
};
typedef int TOKEN_TYPE;

//	----------------------------------------------------------
//	Žš‹åî•ñ
//	----------------------------------------------------------
struct TOKEN {
	std::string text;
	TOKEN_TYPE type;
	TOKEN( std::string txt , TOKEN_TYPE typ ){
		text = txt;
		type = typ;
	}
	int toAssembleCode();
};


//	----------------------------------------------------------
//	Žš‹åî•ñƒ‚ƒWƒ…[ƒ‹
//	----------------------------------------------------------
class LexcialReader {
private :
	size_t m_index;
	std::string m_text;
	std::vector<TOKEN> m_tokens;
public  :
	std::vector<TOKEN> getResult();
	LexcialReader( CStream stream );
	~LexcialReader();
private :
	void _execute();
	void _initialize( CStream stream );
	void _advance();
	void _backstep();
	char _getc();
	bool _isEof();
	void _isS();
	bool _isComment();
	void _isSyntax2Wd();
	void _isSyntax1Wd();
	bool _isLetter();
	bool _isDigit();
	bool _isString();
};
typedef std::shared_ptr<LexcialReader> CLexcialReader;

}
//	namespace SenchaVM
