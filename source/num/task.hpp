#ifndef SCIGMA_NUM_TASK_HPP
#define SCIGMA_NUM_TASK_HPP

#include <functional>

namespace scigma
{
  namespace num
  {

    extern "C" int ESCAPE_COUNT;

    typedef std::function<void(void)> Task; 

    void run_task(void* taskPtr);
    
  }
}

#endif /* SCIGMA_NUM_TASK_HPP */
