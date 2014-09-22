#pragma once

#include "stream.h"

namespace Sencha{
namespace Util {

/*
 * ビットデータ読み込みクラス
 * ストリームをコンストラクタ引数に渡し、ビット単位でデータをシーケンスして取得する。
 * 先頭バイトから読み進めるため、形式はビッグエンディアンにしか対応していない。
 */
class BitReader{
private :
	CStream m_stream;
	int m_index;
public :
	BitReader( CStream stream );
};

} // namespace Util
} // namespace Sencha