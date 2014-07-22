#include "vmassemble_reader.h"
#include "vm_mnemonic_define.h"


namespace SenchaVM{
namespace Assembly{

// ***********************************************************************
// class AssembleReader
// ***********************************************************************
AssembleReader::AssembleReader( CStream stream ){
	VM_ASSERT( stream.get() );
	m_stream = stream;
	m_writer = CBinaryWriter( new BinaryWriter() );
	_read();
}

string AssembleReader::_readLine(){
	string result = "";
	while( m_stream->hasNext() ){
		char c = (char)m_stream->getByte();
		if( c == '#' ){
			while( m_stream->hasNext() ){
				c = m_stream->getByte();
				if( c == '\n' ) break;
				if( c == EOF )  break;
			}
			continue;
		}
		if( c == EOF ){
			break;
		}
		if( c == '\n' ){
			break;
		}
		result += c;
	}
	return result;
}

void AssembleReader::_read(){
	while( m_stream->hasNext() ){
		_analyzeLine( _readLine() );
	}
}
void AssembleReader::_analyzeLine( string line ){
	vector<string> command = _split( line , ":," );
	VM_PRINT( ">> " );
	for( size_t i = 0 ; i < command.size() ; i++ ){
		VM_PRINT( "[%s]," , command[i].c_str() );
	}
	VM_PRINT( "\n" );

	if( command.size() < 1 ){
		return;
	}
	string token = command[0];
	if( token.compare( "def" ) == 0 ){
		m_currentAssembly.setName( command[1] );
		return;
	}
	if( token.compare( "stackframe" ) == 0 ){
		m_currentAssembly.setStackFrame( (int)atoi( command[1].c_str() ) );
		return;
	}
	if( token.compare( "end" ) == 0 ){
		m_writer->write( EMnemonic::EndFunc );
		CStream stream = m_writer->getStream();
		while( stream->hasNext() ){
			m_currentAssembly.pushByte( stream->getByte() );
		}
		stream->clear();
		m_assembleCollection.assemblyInfo.push_back( m_currentAssembly );
		m_currentAssembly.setName( "" );
		m_currentAssembly.clearBytes();
		return;
	}
	if( token.compare( "pmov" ) == 0 ){
		m_writer->write( EMnemonic::PMov );
		string src  = command[1];
		string dest = command[2];
		_writeMemory( src );
		_writeMemory( dest );
		return;
	}
	if( token.compare( "mov" ) == 0 ){
		m_writer->write( EMnemonic::Mov );
		string src  = command[1];
		string dest = command[2];
		_writeMemory( src );
		_writeMemory( dest );
		return;
	}
	if( token.compare( "add" ) == 0 ){
		m_writer->write( EMnemonic::Add );
		string src  = command[1];
		string dest = command[2];
		_writeMemory( src );
		_writeMemory( dest );
		return;
	}
	if( token.compare( "sub" ) == 0 ){
		m_writer->write( EMnemonic::Sub );
		string src  = command[1];
		string dest = command[2];
		_writeMemory( src );
		_writeMemory( dest );
		return;
	}
	if( token.compare( "mul" ) == 0 ){
		m_writer->write( EMnemonic::Mul );
		string src  = command[1];
		string dest = command[2];
		_writeMemory( src );
		_writeMemory( dest );
		return;
	}
	if( token.compare( "div" ) == 0 ){
		m_writer->write( EMnemonic::Div );
		string src  = command[1];
		string dest = command[2];
		_writeMemory( src );
		_writeMemory( dest );
		return;
	}
	if( token.compare( "rem" ) == 0 ){
		m_writer->write( EMnemonic::Rem );
		string src  = command[1];
		string dest = command[2];
		_writeMemory( src );
		_writeMemory( dest );
		return;
	}
	if( token.compare( "call" ) == 0 ){
		m_writer->write( EMnemonic::Call );
		string funcName  = command[1];
		_writeMemory( funcName );
		return;
	}
	if( token.compare( "push" ) == 0 ){
		m_writer->write( EMnemonic::Push );
		string src  = command[1];
		_writeMemory( src );
		return;
	}
}
vector<string> AssembleReader::_split( string text , string clist ){
	vector<string> result;
	string s = "";
	bool isString = false;
	for( size_t index = 0 ; index < text.length() ; index++ ){
		char current = text[index];
		if( current == '\t' || current == ' ' || current == '\n' || current == '\r' ){
			if( !isString ){
				continue;
			}
		}
		if( current == '"' ){
			isString = !isString;
		}

		if( _found( current , clist )){
			result.push_back( s );
			s = "";
			continue;
		}
		s += current;
	}
	result.push_back( s );
	return result;
}
bool AssembleReader::_found( char c , string s ){
	for( size_t i = 0 ; i < s.length() ; i++ ){
		if( c == s[i] ) return true;
	}
	return false;
}

void AssembleReader::_writeMemory( string s ){
	VM_ASSERT( s.length() >= 1 );
	switch( s[0] ){
	case '&' :
		{
			m_writer->writeInt32( _toInt( s , 2 ) );
		}
		break;
	case 'L' : 
		{
			m_writer->write( EMnemonic::MEM_L );
			m_writer->writeInt32( _toInt( s , 1 ) );
//			VM_PRINT( "MEM_L\n" );
		}
		break;
	case 'S' :
		{
			m_writer->write( EMnemonic::MEM_S );
			m_writer->writeInt32( _toInt( s , 1 ) );
//			VM_PRINT( "MEM_S\n" );
		}
		break;
	case 'R' :
		{
			m_writer->write( EMnemonic::REG );
			m_writer->writeInt32( _toInt( s , 1 ) );
//			VM_PRINT( "MEM_R\n" );
		}
		break;
	case '"' :
		{
			string value = _toString( s , 0 );
			m_writer->write( EMnemonic::LIT_STRING );
			m_writer->writeString( value );
//			VM_PRINT( "LIT_STRING %s \n" , value.c_str() );
		}
		break;
	default :
		{
			double value = _toDouble( s , 0 );
			m_writer->write( EMnemonic::LIT_VALUE );
			m_writer->writeDouble( value );
//			VM_PRINT( "LIT_VALUE %0.2f \n" , value );
		}
		break;
	}
}
int AssembleReader::_toInt( string s , int offset ){
	string value = "";
	for( size_t i = offset ; i < s.length() ; i++ ){
		value += s[i];
	}
	return atoi( value.c_str() );
}
string AssembleReader::_toString( string s , size_t offset ){
	if( offset >= s.length() ) return "";
	if( s[offset] != '"' )     return "";
	string value = "";
	for( size_t i = 1 + offset ; i < s.length() ; i++ ){
		if( s[i] == '"' ){
			break;
		}
		value += s[i];
	}
	return value;
}
double AssembleReader::_toDouble( string s , int offset ){
	string value = "";
	for( size_t i = offset ; i < s.length() ; i++ ){
		value += s[i];
	}
	return atof( value.c_str() );
}


} // namespace Assembly
} // namespace SenchaVM