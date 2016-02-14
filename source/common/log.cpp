#include <cstdio>
#include "log.hpp"

namespace scigma
{
  namespace common
  {

    Log::Log():PythonObject<Log>(this), file_("")
    {}    

    Log::Log(std::string fileName):PythonObject<Log>(this),file_(fileName)
    {
      if(""!=file_)
      remove(file_.c_str());
    }
    
    const char* Log::strip_path(const char* file)
    {
      size_t found = std::string(file).find_last_of("/\\");
      if(std::string::npos!=found)
	return &file[found+1];
      return file;
    }
    
  } /* end namespace common */
} /* end namespace scigma */
