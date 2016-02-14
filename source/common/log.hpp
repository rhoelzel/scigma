#ifndef SCIGMA_COMMMON_LOG_HPP
#define SCIGMA_COMMMON_LOG_HPP

#include <string>
#include <sstream>
#include <deque>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#include <tinythread.h>
#pragma clang diagnostic pop
#include "pythonobject.hpp"

namespace scigma
{
  namespace common
  {
    class Log:public PythonObject<Log>
    {
    public:
      
      enum Type
	{
	  LOG_SUCCESS=0,
	  LOG_FAIL=1,
	  LOG_DATA=2,
	  LOG_WARNING=3,
	  LOG_ERROR=4,
	  LOG_DEFAULT=5
	};

      Log();
      Log(std::string fileName);
      
      template <Type T=LOG_DEFAULT> void push(const std::string& text, const char* file=NULL, int line=0)
      {
	if(text=="")
	  return;
	
	std::ostringstream combine;
	if(file)
	  combine<<strip_path(file)<<", "<<"line "<<line<<": "<<text; 
	else
	  combine<<text;
	
	tthread::lock_guard<tthread::mutex> guard(mutex_);
	list_[T].push_back(combine.str());
	if(""!=file_)
	  {
	    FILE * pFile;
	    pFile = fopen (file_.c_str(), "a" );
	    if (pFile!=NULL)
	      {
		fprintf(pFile,"%s\n",combine.str().c_str());
		fclose(pFile);
	      }
	  }
      }
      
      template <Type T=LOG_DEFAULT> std::string pop()
      {
	tthread::lock_guard<tthread::mutex> guard(mutex_);
	if(!list_[T].empty())
	  {
	    std::string(result)(list_[T].front());
	    list_[T].pop_front();
	    return result;
	  }
	return "";
      }
      
    private:
      
      Log(const Log&);
      Log& operator=(const Log&);
      const char* strip_path(const char* file);
      
      std::deque<std::string> list_[LOG_DEFAULT+1];
      tthread::mutex mutex_;
      std::string file_;
    };

  } /* end namespace common */
} /* end namespace scigma */

#endif /* SCIGMA_COMMON_LOG_HPP */
