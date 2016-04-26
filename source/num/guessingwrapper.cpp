#include "../common/log.hpp"
#include "../common/blob.hpp"
#include "../dat/wave.hpp"
#include "guessing.hpp"
#include "iteration.hpp"
#include "equationsystem.hpp"
#include <iostream>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"

using namespace scigma::common;
using namespace scigma::num;
using namespace scigma::dat;

extern "C"
{
  void* scigma_num_guess(const char* identifier, PythonID equationSystemID,PythonID logID,
			 PythonID varyingWaveID, PythonID evWaveID, PythonID blobID,
			 bool showAllIterates, bool noThread)
  {
    PYOBJ(EquationSystem,eqsys,equationSystemID);if(!eqsys)return NULL;
    PYOBJ(Log,log,logID);if(!log)return NULL;
    PYOBJ(Wave,varyingWave,varyingWaveID);if(!varyingWave)return NULL;
    PYOBJ(Wave,evWave,evWaveID);if(!evWave)return NULL;
    PYOBJ(Blob,blob,blobID);if(!blob)return NULL;
    
    Mode m((Mode(blob->get_int("mode"))));
    double period(blob->get_double("period"));
    size_t nPeriod((size_t(blob->get_int("nperiod"))));
    double dt(blob->get_double("dt"));
    double maxtime(blob->get_double("maxtime"));
    int secvar(blob->get_int("secvar"));
    int secdir(blob->get_int("secdir"));
    double secval(blob->get_double("secval"));
    bool jac(blob->get_int("odessa.Jacobian")==SYMBOLIC?true:false);
    bool stiff(blob->get_string("odessa.type")=="stiff"?true:false);
    double aTol(blob->get_double("odessa.atol"));
    double rTol(blob->get_double("odessa.rtol"));
    size_t maxIter((size_t(blob->get_int("odessa.mxiter"))));
    double nTol(blob->get_double("Newton.tol"));
    bool nJac(blob->get_int("Newton.Jacobian")==SYMBOLIC?true:false);

    size_t nVar(eqsys->n_variables());
    size_t nFunc(eqsys->n_functions());
    
    Newton::F* f(NULL);
    std::function<void(const double*, double*)>* ff(NULL);
    
    Stepper* stepper(NULL);

    
    Task* task(NULL);
    
    switch(m)
      {
      case MAP:
	stepper=create_map_stepper(*eqsys,true);
	task=create_guessing_task(identifier,log,stepper,varyingWave,evWave,nTol,size_t(nPeriod),showAllIterates?1:0,-1);
	/*	f=create_map_newton_function(*eqsys,size_t(nPeriod),jac);
		ff=create_additional_function_evaluator(*eqsys);
		task=create_guessing_task(identifier,log,nVar,nFunc,f,ff,varyingWave,evWave,nJac,nTol,true);*/
	break;
      case ODE:
	if(!eqsys->is_autonomous())
	  {
	    log->push<Log::ERROR>("cannot use Newton on non-autonomous dynamical system\n");
	    return NULL;
	  }
	f=create_ode_newton_function(*eqsys,nJac);
	ff=create_additional_function_evaluator(*eqsys);
	task=create_guessing_task(identifier,log,nVar,nFunc,f,ff,varyingWave,evWave,nJac,nTol,false);
	break;
      case STROBE:
	stepper=create_ode_stepper(*eqsys,true,period,jac,stiff,aTol,rTol,size_t(maxIter));
	task=create_guessing_task(identifier,log,stepper,varyingWave,evWave,nTol,size_t(nPeriod),showAllIterates?1:0,-1);
	break;
      case POINCARE:
	stepper=create_poincare_stepper(*eqsys,true,maxtime,dt,secvar,secdir,secval,nTol,jac,stiff,aTol,rTol,size_t(maxIter));
	task=create_guessing_task(identifier,log,stepper,varyingWave,evWave,nTol,size_t(nPeriod),showAllIterates?1:0,long(secvar));
      }
    
    if(!noThread)
      {
	(*task)();
	delete task;
	return NULL;
      }
    else
      {
	return reinterpret_cast<void*>(new tthread::thread(run_task,reinterpret_cast<void*>(task)));
      }
  }

} /* end extern "C" block */

#pragma clang pop
