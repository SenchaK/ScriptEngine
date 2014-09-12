#include "sencha_vm.h"


namespace Sencha {
namespace VM {


SenchaVM::SenchaVM(){
	this->m_lexer = NULL;
	this->m_reader = NULL;
	this->m_driver = NULL;
	this->m_log = NULL;
	this->m_built_in = new VMBuiltIn();
}

SenchaVM::~SenchaVM(){
	if( this->m_lexer )   { delete this->m_lexer   ; this->m_lexer    = NULL; }
	if( this->m_reader )  { delete this->m_reader  ; this->m_reader   = NULL; }
	if( this->m_driver )  { delete this->m_driver  ; this->m_driver   = NULL; }
	if( this->m_log )     { delete this->m_log     ; this->m_log      = NULL; }
	if( this->m_built_in ){ delete this->m_built_in; this->m_built_in = NULL; }
}

void SenchaVM::clear(){
	if( this->m_lexer ) { delete this->m_lexer ; this->m_lexer  = NULL; }
	if( this->m_reader ){ delete this->m_reader; this->m_reader = NULL; }
	if( this->m_driver ){ delete this->m_driver; this->m_driver = NULL; }
	if( this->m_log )   { delete this->m_log   ; this->m_log    = NULL; }
}

void SenchaVM::log_init( Log* logger ){
	if( this->m_log ){
		delete this->m_log;
	}
	this->m_log = logger;
}

void SenchaVM::cleanup(){
	this->clear();
	delete this->m_built_in;
	this->m_built_in = new VMBuiltIn();
}

void SenchaVM::compile_from_text( string text ){
	assert( !this->m_lexer );
	assert( !this->m_reader );
	assert( !this->m_driver );
	this->m_lexer = new Lexer( CStream( new TextStream( text ) ) );
	this->m_reader = new Parser( this->m_lexer , this->m_built_in , this->m_log );
	this->m_driver = new VMDriver( this->m_reader , this->m_built_in );
}

void SenchaVM::compile_from_file( const char* fileName ){
	assert( !this->m_lexer );
	assert( !this->m_reader );
	assert( !this->m_driver );
	this->m_lexer = new Lexer( CStream( new FileStream( fileName ) ) );
	this->m_reader = new Parser( this->m_lexer , this->m_built_in , this->m_log );
	this->m_driver = new VMDriver( this->m_reader , this->m_built_in );
}

void SenchaVM::compile_from_o_file( const char* objectFileName ){
	assert( !this->m_lexer );
	assert( !this->m_reader );
	assert( !this->m_driver );
	this->m_reader = new VMAssembleInput( CStream( new FileStream( objectFileName ) ) );
	this->m_driver = new VMDriver( this->m_reader , this->m_built_in );
}

void SenchaVM::create_o_file( const char* objectFileName ){
	assert( this->m_reader );
	VMAssembleOutput output( this->m_reader , objectFileName );
}

void SenchaVM::create_text_file_log( const char* fileName ){
	assert( this->m_reader );
	VMAssembleLog log( this->m_reader , CLog( new TextFileLog( fileName ) ) );
}

void SenchaVM::output_console_log(){
	assert( this->m_reader );
	VMAssembleLog log( this->m_reader , CLog( new ConsoleLog() ) );
}

void SenchaVM::define_function( string mappingName , void (*function)(VMDriver*) ){
	this->m_built_in->entryFunction( new VMBuiltInFunction( mappingName , function ) );
}

void SenchaVM::execute_function( string funcName ){
	assert( this->m_driver );
	this->m_driver->executeFunction( funcName );
}



} // namespace VM
} // namespace Sencha
