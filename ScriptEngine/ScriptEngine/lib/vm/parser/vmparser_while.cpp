#define WHILE_DEBUG
#ifdef WHILE_DEBUG
	#define WHILE_LOG VM_PRINT
	#define WHILE_ASSERT VM_ASSERT

//	#define WHILE_LOG printf
//	#define WHILE_ASSERT VM_ASSERT
#else
	#define WHILE_LOG(...)
	#define WHILE_ASSERT(...)
#endif


#include "vmparser.h"
#include "..\lexer\vmlexer.h"
#include "..\vmassembler.h"
#include "..\symbol\vmsymbol.h"


namespace SenchaVM {
namespace Assembly {
// **************************************************************************
// while文解析
// 文法チェックを行い計算式評価。
// 評価結果が0の場合は終了チャンクまでジャンプ
// そうでない場合はジャンプしない
// チャンク終了地点に終了地点までのジャンプを入れる
// **************************************************************************
void Parser::_parse_while( ParseParameter* param ){
	WHILE_LOG( ">>while Log\n" );
	WHILE_ASSERT( getToken().type == TokenType::While );  nextToken();
	WHILE_ASSERT( getToken().type == TokenType::Lparen ); nextToken();


	// ループ終了条件
	int whileContinuePos = m_writer->count();
	_expression( param );

	WHILE_ASSERT( getToken().type == TokenType::Rparen );
	WHILE_LOG( ">>while Log 終了\n" );
	
	// 比較演算結果が0の場合終了チャンクまでジャンプする命令を設置
	m_writer->write( EMnemonic::JumpZero );
	int whileJmpPos = m_writer->count();
	m_writer->writeInt32( 0 );

	nextToken();
	ParseParameter whileParam;

	WHILE_LOG( ">>while Log 内部処理解析へ...\n" );
	_parse( &whileParam );
	WHILE_LOG( ">>while Log 内部処理解析終了 %s \n" , getToken().text.c_str() );

	m_writer->write( EMnemonic::Jmp );
	m_writer->writeInt32( whileContinuePos );

	// ループ終了位置を書き込み
	m_writer->writeInt32( m_writer->count() , whileJmpPos );

	// continue,break文を見つけたらジャンプアドレスを設定
	for( size_t i = 0 ; i < whileParam.breakAddr.size() ; i++ ){ m_writer->writeInt32( m_writer->count() , whileParam.breakAddr[i].codeAddr ); }
	for( size_t i = 0 ; i < whileParam.continueAddr.size() ; i++ ){ m_writer->writeInt32( whileContinuePos , whileParam.continueAddr[i].codeAddr ); }
}


} // namespace Assembly
} // namespace SenchaVM
