#pragma once

#include "parser\vmparser.h"
#include "vmregister.h"
#include "vmassembler.h"

namespace SenchaVM {
	SenchaVM::Assembly::CVMMainRoutineDriver CompileFromText( string text );
	SenchaVM::Assembly::CVMMainRoutineDriver OpenFromFile( string fileName );
}
