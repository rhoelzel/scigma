#include "task.hpp"

namespace scigma
{
  namespace num
  {

    void run_task(void* taskPtr)
    {
      Task* task(reinterpret_cast<Task*>(taskPtr));
      (*task)();
      delete task;
    }

  }
}
