#include "util.hpp"

namespace scigma
{
  namespace common
  {
    
    void substring(std::string& subject, const std::string& search,const std::string& replace)
    {
      size_t pos = 0;
      while((pos = subject.find(search, pos)) != std::string::npos) {
	subject.replace(pos, search.length(), replace);
	pos += replace.length();
      }
    }

  } /* end namespace common */
} /* end namespace scigma */
