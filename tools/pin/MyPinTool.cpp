#include <iostream>
#include <map>
#include <set>
#include <assert.h>
#include <fstream>
#include <stdio.h>
#include <sstream>
#include <cmath>
#include <queue>
#include <list>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cinttypes>
#include "pin.H"
#include "control_manager.H"
#include "portability.H"
#include <iomanip>

using namespace std;
using namespace CONTROLLER;

#define SUCCESS_THRESHOLD 10000
/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

UINT64 iteration_no=0;
UINT64 iteration_success=0;


//----------------------------------
//Function declaration
//----------------------------------

VOID offload_funcAfter(ADDRINT ret);
VOID offload_funcBefore();


/* ===================================================================== */

VOID Fini(INT32 code, VOID *v)
{
    cout << "Num Invocations : " << iteration_no << "\n";
    cout << "Num Success Invocations : " << iteration_success << "\n";
      cout<<"Exiting ...\n";
}


VOID offload_funcAfter(ADDRINT ret)
{

  {
    //if offload function is successful - then let ooo know that offload starts
    //and dump all ld st to axc mem file
    if(ret)
    {
      ++iteration_success;

      //flush to axc mem file
      if(iteration_success > SUCCESS_THRESHOLD) {
          Fini(0,0);
          PIN_ExitProcess(0);
      }
    }
    }
}


VOID offload_funcBefore()
{
  iteration_no++;
}

/* ===================================================================== */
/* Instrumentation routines                                              */
/* ===================================================================== */

VOID Image(IMG img, VOID *v)
{
    for( SEC sec= IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec) )
    {
        for( RTN rtn= SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn) )
        {
            string rtnName =  PIN_UndecorateSymbolName(RTN_Name(rtn), UNDECORATION_NAME_ONLY);
            if (rtnName.find(string("__offload_func")) != string::npos) {
                if (RTN_Valid(rtn))
                  {
                    RTN_Open(rtn);
                    RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)offload_funcAfter,
                        IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);
                    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)offload_funcBefore,
                        IARG_END);
                    RTN_Close(rtn);
                  }
            }
        }
    }

}





/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
  cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
  return -1;
}






/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{
  // Initialize pin & symbol manager
  PIN_InitSymbols();
  if( PIN_Init(argc,argv) )
  {
    return Usage();
  }

  // Register Image to be called to instrument functions.
  IMG_AddInstrumentFunction(Image, 0);
  PIN_AddFiniFunction(Fini, 0);


  // Never returns
  PIN_StartProgram();

  return 0;
}

