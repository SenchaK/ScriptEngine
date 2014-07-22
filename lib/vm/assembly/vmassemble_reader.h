#pragma once
#include "../vmdefine.h"
#include "vmassembly_info.h"

namespace SenchaVM{
namespace Assembly{
class AssembleReader;
typedef shared_ptr<AssembleReader> CAssembleReader;


// ストリームから中間言語を適切なバイナリデータに置き換える
class AssembleReader {
private :
	AssemblyInfo m_currentAssembly;
	VMAssembleCollection m_assembleCollection;
	CStream m_stream;
	CBinaryWriter m_writer;
public  :
	AssembleReader( CStream stream );
	const VMAssembleCollection& getResult(){ 
		return m_assembleCollection;
	}
private :
	string _readLine();
	void _read();
	void _analyzeLine( string line );
	vector<string> _split( string text , string clist );
	void _writeMemory( string s );
	int _toInt( string s , int offset );
	string _toString( string s , size_t offset );
	double _toDouble( string s , int offset );
	bool _found( char c , string s );
};


} // namespace Assembly
} // namespace SenchaVM