#include <string>
#include "log.hpp"

using namespace scigma::common;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wmissing-variable-declarations"

extern "C"
{
  // wrappers for the Log class
  PythonID scigma_create_log(){Log* ptr=new Log("");return ptr->get_python_id();}
  void scigma_destroy_log(PythonID objectID){PYOBJ(Log,ptr,objectID);if(ptr)delete ptr;}

  std::string logMessage;
  
  const char* scigma_common_log_pop(PythonID objectID, Log::Type type)
  {
    PYOBJ(Log,ptr,objectID);
    if(ptr)
      {
	switch(type)
	  {
	  case Log::LOG_SUCCESS:
	    logMessage=ptr->pop<Log::LOG_SUCCESS>();
	    break;
	  case Log::LOG_FAIL:
	    logMessage=ptr->pop<Log::LOG_FAIL>();
	    break;
	  case Log::LOG_DATA:
	    logMessage=ptr->pop<Log::LOG_DATA>();
	    break;	    
	  case Log::LOG_WARNING:
	    logMessage=ptr->pop<Log::LOG_WARNING>();
	    break;
	  case Log::LOG_ERROR:
	    logMessage=ptr->pop<Log::LOG_ERROR>();
	    break;
	  case Log::LOG_DEFAULT:
	    logMessage=ptr->pop<Log::LOG_DEFAULT>();
	    break;
	  }
	return logMessage.c_str();
      }
    else
      return NULL;
  }

} /* end extern "C" block */

#pragma clang diagnostic pop
