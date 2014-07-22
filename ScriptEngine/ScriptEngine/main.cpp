#include "lib\vm\sencha_vm.h"
#include "vmplugin\VMFileSystem.h"
#include "vmplugin\VMSystem.h"

using namespace SenchaVM;
using namespace SenchaVM::Assembly;
class DefaultSystemCall : public VMSystemCallService {
public :
	virtual void onInit( VMDriver* driver ){
		VMSystemCallService::onInit( driver );
		this->addSystemCallService( new FileSystemLibrary() );
		this->addSystemCallService( new VMSystem() );
	}
};

void main(){
	CVMMainRoutineDriver driver = OpenFromFile( "sample/FizzBuzz.txt" );
	driver->setSystemCallService( CVMSystemCallService( new DefaultSystemCall() ) );
	driver->executeFunction( "main" );
}
