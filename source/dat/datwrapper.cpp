#include <vector>
#include "mesh.hpp"
#include "wave.hpp"

using namespace scigma::common;
using namespace scigma::dat;

typedef AbstractWave<double> Wave;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"

extern "C"
{
  /* Wrappers for Wave */
  
  PythonID scigma_dat_create_wave(int capacity)
  {
    Wave* ptr(NULL);
    if(capacity>0)
      ptr=new Wave(size_t(capacity));
    else
      ptr=new Wave;
    return ptr->get_python_id();
  }
  void scigma_dat_destroy_wave(PythonID objectID)
  {PYOBJ(Wave,ptr,objectID);if(ptr){delete ptr;}}


  void scigma_dat_wave_push_back(PythonID objectID, double* values, int nValues)
  {
    PYOBJ(Wave,ptr,objectID);
    if(ptr)
      ptr->push_back(values,size_t(nValues));
  }

  void scigma_dat_wave_pop_back(PythonID objectID, int nValues)
  {
    PYOBJ(Wave,ptr,objectID);
    if(ptr)
      ptr->pop_back(size_t(nValues));
  }
  
  void scigma_dat_wave_lock(PythonID objectID)
  {
    PYOBJ(Wave,ptr,objectID);
    if(ptr)
      ptr->lock();
  }

  void scigma_dat_wave_unlock(PythonID objectID)
  {
    PYOBJ(Wave,ptr,objectID);
    if(ptr)
      ptr->unlock();
  }

  /* use very carefully, only between calls 
     to lock() and unlock()
  */
  int scigma_dat_wave_size(PythonID objectID)
  {
    PYOBJ(Wave,ptr,objectID);
    if(!ptr)
      return -1;
    return int(ptr->size());
  }
  
  /* use very carefully, access resulting pointer 
     only between calls to lock() and unlock()
  */
  double* scigma_dat_wave_data(PythonID objectID)
  {
    PYOBJ(Wave,ptr,objectID);
    if(ptr)
      return ptr->data();
    else
      return NULL;
  }

  /* Wrappers for Mesh */

  PythonID scigma_dat_create_mesh(int nDim, int nInitial, double* initial)
  {
    std::vector<double> vInitial;
    for(size_t i(0);i<size_t(nInitial*nDim);++i)
      vInitial.push_back(initial[i]);
    Mesh* ptr=new Mesh(size_t(nDim),vInitial);
    return ptr->get_python_id();
  }
  void scigma_dat_destroy_mesh(PythonID objectID)
  {PYOBJ(Mesh,ptr,objectID);if(ptr){delete ptr;}}


} /* end extern "C" block */

#pragma clang diagnostic pop

#undef PYOBJ
