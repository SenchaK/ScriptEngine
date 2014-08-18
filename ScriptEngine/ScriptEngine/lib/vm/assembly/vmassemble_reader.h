#pragma once
#include "../vmdefine.h"
#include "vmassembly_info.h"
#include "../parser/vmparser.h"

namespace SenchaVM{
namespace Assembly{
class AssembleReader;
typedef shared_ptr<AssembleReader> CAssembleReader;


class AssembleReader {
public  :
	AssembleReader( CParser* parser );
};


} // namespace Assembly
} // namespace SenchaVM