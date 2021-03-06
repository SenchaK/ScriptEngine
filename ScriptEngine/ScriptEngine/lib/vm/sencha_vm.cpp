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

/*
 * デストラクタ
 * 全てのメモリを解放
 */
SenchaVM::~SenchaVM(){
	if( this->m_lexer )   { delete this->m_lexer   ; this->m_lexer    = NULL; }
	if( this->m_reader )  { delete this->m_reader  ; this->m_reader   = NULL; }
	if( this->m_driver )  { delete this->m_driver  ; this->m_driver   = NULL; }
	if( this->m_log )     { delete this->m_log     ; this->m_log      = NULL; }
	if( this->m_built_in ){ delete this->m_built_in; this->m_built_in = NULL; }
	this->execFinalize();
}

/*
 * エンジン部分を動かすのに必要なものだけ開放
 */
void SenchaVM::clear(){
	if( this->m_lexer ) { delete this->m_lexer ; this->m_lexer  = NULL; }
	if( this->m_reader ){ delete this->m_reader; this->m_reader = NULL; }
	if( this->m_driver ){ delete this->m_driver; this->m_driver = NULL; }
	if( this->m_log )   { delete this->m_log   ; this->m_log    = NULL; }
}

/*
 *
 */
void SenchaVM::log_init( Log* logger ){
	if( this->m_log ){
		delete this->m_log;
	}
	this->m_log = logger;
}

/*
 * デストラクタ時にのみ呼ばれる
 */
void SenchaVM::execFinalize(){
	for( list<event_handler>::iterator iter = this->m_event.begin() ; iter != this->m_event.end() ; iter++ ){
		iter->finalize();
	}
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
	this->m_driver = new Subroutine( this->m_reader , this->m_built_in );
}

void SenchaVM::compile_from_file( const char* fileName ){
	assert( !this->m_lexer );
	assert( !this->m_reader );
	assert( !this->m_driver );
	this->m_lexer = new Lexer( CStream( new FileStream( fileName ) ) );
	this->m_reader = new Parser( this->m_lexer , this->m_built_in , this->m_log );
	this->m_driver = new Subroutine( this->m_reader , this->m_built_in );
}

void SenchaVM::compile_from_o_file( const char* objectFileName ){
	assert( !this->m_lexer );
	assert( !this->m_reader );
	assert( !this->m_driver );
	this->m_reader = new VMAssembleInput( CStream( new FileStream( objectFileName ) ) );
	this->m_driver = new Subroutine( this->m_reader , this->m_built_in );
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

void SenchaVM::execute(){
	assert( this->m_driver );
	this->m_driver->execute();
}

void SenchaVM::on_update(){
	assert( this->m_driver );
	this->m_driver->OnUpdate();
}

Package* SenchaVM::insert_package( string packageName ){
	return this->m_built_in->insertPackage(packageName);
}

Memory* SenchaVM::L( int addr ){
	return &this->m_driver->getMemory( EMnemonic::MEM_L , addr );
}

void SenchaVM::add_event( event_handler evt ){
	assert( evt.init );
	assert( evt.finalize );
	evt.init();
	m_event.push_back( evt );
}

int SenchaVM::BP(){
	return this->m_driver->baseP();
}
int SenchaVM::SP(){
	return this->m_driver->sp();
}
int SenchaVM::PushCount(){
	return this->m_driver->pushCount();
}
void SenchaVM::setBreakPoint( string funcName , int pc ){
	this->m_driver->setBreakPoint( funcName , pc );
}
void SenchaVM::setBreakPoint( int funcAddr , int pc ){
	this->m_driver->setBreakPoint( funcAddr , pc );
}

} // namespace VM
} // namespace Sencha
