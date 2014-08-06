#include "vmerror.h"

namespace SenchaVM {
namespace Assembly {

/*
 * エラーＩＤ列挙値を文字列にして返す
 * @return 該当するＩＤの名前を返す。存在しない場合OTHER_ERRORを返す
 */
string ERR_ID_EX::toString( ERR_ID value ){
	switch( value ){
	case ERROR_1001 : return "ERROR_1001";
	case ERROR_1002 : return "ERROR_1002";
	case ERROR_2001 : return "ERROR_2001";
	case ERROR_2002 : return "ERROR_2002";
	case ERROR_2003 : return "ERROR_2003";
	case ERROR_3001 : return "ERROR_3001";
	case ERROR_3002 : return "ERROR_3002";
	case ERROR_4001 : return "ERROR_4001";
	case ERROR_C2065 : return "ERROR_C2065";
	case ERROR_C2143 : return "ERROR_C2143";
	}
	return "OTHER_ERROR";
}

/* 
 * コンストラクタ
 * エラー情報に合わせてメッセージを生成する
 * 
 * @param param 
 * エラー情報の含まれたオブジェクト。
 * キャストして使用する。
 * 必ずパラメータは含まれていないといけない。
 * パラメータは必ずnewで生成すること
 */
VMError::VMError( VMErrorParameter* param ){
	assert( param );

	ostringstream o;
	switch( param->errId ){
	case ERROR_3001 :
		{
			ERROR_INFO_3001* errinfo = reinterpret_cast<ERROR_INFO_3001*>( param );
			o << "構造体スコープ内で関数を宣言することはできません [funcName " << errinfo->funcName << "]";
		}
		break;
	case ERROR_4001 :
		{
			ERROR_INFO_4001* errinfo = reinterpret_cast<ERROR_INFO_4001*>( param );
			o << "不明なシンボルタイプ" << "[symbolName " << errinfo->symbolName << "][symbolType " << errinfo->symbolType << "]";
		}
		break;
	case ERROR_C2065 :
		{
			ERROR_INFO_C2065* errinfo = reinterpret_cast<ERROR_INFO_C2065*>( param );
			o << "\"" << errinfo->symbolName << "\":" << "定義されていない識別子です。";
		}
		break;
	case ERROR_C2143 :
		{
			ERROR_INFO_C2143* errinfo = reinterpret_cast<ERROR_INFO_C2143*>( param );
			o << "構文エラー : " << errinfo->tokenType << "ありません。";
		}
		break;
	case ERROR_C2059 :
		{
			ERROR_INFO_C2059* errinfo = reinterpret_cast<ERROR_INFO_C2059*>( param );
			o << "構文エラー : \"" << errinfo->s << "\"";
		}
		break;
	}

	this->m_message = ERR_ID_EX::toString( param->errId ) + ":" + o.str();
	this->m_param = param;
}

/*
 * デストラクタ
 * パラメータインスタンスを解放する
 */
VMError::~VMError(){
	assert( m_param );
	if( m_param ){
		delete m_param;
	}
}


} // namespace Assembly
} // namespace SenchaVM