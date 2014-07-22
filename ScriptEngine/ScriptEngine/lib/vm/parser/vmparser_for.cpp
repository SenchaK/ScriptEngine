#define FOR_DEBUG
#ifdef FOR_DEBUG
	#define FOR_LOG    VM_PRINT
	#define FOR_ASSERT VM_ASSERT

//	#define FOR_LOG    printf
//	#define FOR_ASSERT assert
#else
	#define FOR_LOG(...)
	#define FOR_ASSERT(...)
#endif

#include "vmscope.h"
#include "vmparser.h"
#include "..\lexer\vmlexer.h"
#include "..\vmassembler.h"
#include "..\symbol\vmsymbol.h"


namespace SenchaVM {
namespace Assembly {

// **************************************************************************
// for文解析
// 3箇所の計算を行う
//		1.最初処理
//		2.終了条件
//		3.ループ終了時処理
// 評価結果が0の場合は終了チャンクまでジャンプ
// そうでない場合はジャンプしない
// チャンク終了地点に開始地点までのジャンプを入れる
// **************************************************************************
void Parser::_parse_for( ParseParameter* param ){
	m_currentScope = m_currentScope->goToChildScope( "__for__" );

	FOR_LOG( ">>for Log\n" );
	FOR_ASSERT( getToken().type == TokenType::For );    nextToken();
	FOR_ASSERT( getToken().type == TokenType::Lparen ); nextToken();
	// 最初の処理
	_parse( param );

	// ループ終了条件
	int forContinuePos = m_writer->count();
	_expression( param );
	
	FOR_ASSERT( getToken().type == TokenType::Semicolon );
	// 比較演算結果が0の場合終了チャンクまでジャンプする命令を設置
	m_writer->write( EMnemonic::JumpZero );
	int forJmpPos = m_writer->count();
	m_writer->writeInt32( 0 );

	nextToken();

	// 最後処理
	// 位置を記憶しておいて飛ばす
	int lastExpressionPos = this->m_pos;
	_skipParen();
	FOR_ASSERT( getToken().type == TokenType::Rparen );

	// ')'を飛ばす
	nextToken();
	// 次の１命令を解析(チャンクがある場合は終了チャンクまでそのまま解析するようにする)
	ParseParameter forParam;

	FOR_LOG( ">>for Log 内部処理解析開始\n" );
	_parse( &forParam );
	FOR_LOG( ">>for Log 内部処理解析終了 [%s]\n" , getToken().text.c_str() );

	int processEndPos = this->m_pos;

	// 位置を戻して最後の計算式を評価
	this->m_pos = lastExpressionPos;
	int loopEndPos = m_writer->count();
	_expression( param );

	// 最後の計算が終わったら元の位置に戻す
	m_pos = processEndPos;

	// ループ終了条件式位置までジャンプ
	m_writer->write( EMnemonic::Jmp );
	m_writer->writeInt32( forContinuePos );

	// ループ終了位置を書き込み
	m_writer->writeInt32( m_writer->count() , forJmpPos );

	m_currentScope = m_currentScope->backToChildScope();
	// continue,break文を見つけたらジャンプアドレスを設定
	// for文ではcontinue時に最後処理をしてからループ先頭位置に戻す点に注意
	for( size_t i = 0 ; i < forParam.breakAddr.size() ; i++ ){ m_writer->writeInt32( m_writer->count() , forParam.breakAddr[i].codeAddr ); }
	for( size_t i = 0 ; i < forParam.continueAddr.size() ; i++ ){ m_writer->writeInt32( loopEndPos , forParam.continueAddr[i].codeAddr ); }
}

} // namespace Assembly
} // namespace SenchaVM