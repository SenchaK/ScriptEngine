#pragma once

#include "../util/logger.h"
#include "vmdefine.h"

#include "parser\vmparser.h"
#include "vmregister.h"
#include "vmassembler.h"

#include "assembly\vm_assemble_log.h"


namespace SenchaVM {
	SenchaVM::Assembly::CVMMainRoutineDriver CompileFromText( string text );
	SenchaVM::Assembly::CVMMainRoutineDriver OpenFromFile( string fileName );

	namespace Assembly {
		void VMAssembleTextFileLog( IAssembleReader* reader , const char* fileName );
		void VMAssembleConsoleLog( IAssembleReader* reader );
	}
}
