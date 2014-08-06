#define IF_DEBUG
#ifdef IF_DEBUG
	#define IF_LOG VM_PRINT
	#define IF_ASSERT VM_ASSERT
#else
	#define IF_LOG(...)
	#define IF_ASSERT(...)
#endif

#include "vmparser.h"
#include "..\lexer\vmlexer.h"
#include "..\vmassembler.h"
#include "..\symbol\vmsymbol.h"

namespace SenchaVM {
namespace Assembly {

// **************************************************************************
// if文解析
// 文法チェックを行い計算式評価。
// 評価結果が0の場合はelse文発見までジャンプ
// そうでない場合はジャンプしない
// チャンク終了地点に終了地点までのジャンプを入れる
// **************************************************************************
void Parser::_parse_if( Context* param ){
	IF_LOG( "**if文\n" );
	IF_ASSERT( getToken().type == TokenType::If );     nextToken();
	IF_ASSERT( getToken().type == TokenType::Lparen ); nextToken();
//	_expression( param );
	IF_ASSERT( getToken().type == TokenType::Rparen );
	
	// 比較演算結果が0の場合else発見場所までジャンプする命令を設置
	// この段階ではジャンプ先アドレスがわからないので0を入れておき、書き込み位置を記録しておいて後で上書きする
	m_writer->write( EMnemonic::JumpZero );
	int elseJmpPos = m_writer->count();
	m_writer->writeInt32( 0 );
	//
	// 次の１命令を解析(チャンクがある場合は終了チャンクまでそのまま解析するようにする)
	nextToken();
	_parse( param );
	// 
	// 終了チャンクまでジャンプさせる命令を設置
	// ここからelse文検索
	// else ifの場合は_parse_ifを呼び出す
	IF_LOG( "**if文終了\n" );
	m_writer->writeInt32( m_writer->count() , elseJmpPos );

	IF_LOG( "**else探し : %s \n" , getToken().text.c_str() );
	while( getToken().type == TokenType::Else ){
		// if文終了してelseがあるなら終了地点までジャンプさせる命令を入れる
		m_writer->write( EMnemonic::Jmp );
		int ifEndJmpPos = m_writer->count();
		m_writer->writeInt32( 0 );
		m_writer->writeInt32( m_writer->count() , elseJmpPos );

		// elseの次に勧めて解析
		nextToken();
		_parse( param );

		// 現在の位置をジャンプ先アドレスとする
		m_writer->writeInt32( m_writer->count() , ifEndJmpPos );
	}
	IF_LOG( "**else探索終了 : %s \n" , getToken().text.c_str() );
}

} // namespace Assembly
} // namespace SenchaVM

