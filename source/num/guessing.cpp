#include "guessing.h"
#include "eigen.h"
#include "functionwrappers.h"
#include <iostream>

namespace scigma
{
  namespace num
  {

    Newton::F* create_map_newton_function(const EquationSystem& eqsys, size_t(nPeriod), bool extJac)
    {
      if(eqsys.is_internal())
	{
	  Function tFunc;
	  VecF xFuncs,rhsFuncs,funcFuncs;
	  eqsys.detach(tFunc,xFuncs,rhsFuncs,funcFuncs);
	  size_t nVar(xFuncs.size());
	  
	  VecF jacobian;
	  build_partial_derivative_matrix(xFuncs,rhsFuncs,jacobian);
	  return new Newton::F
	    ([=](const double* x, double* rhs) mutable
	     {
	       /*	       VecD work1(nVar*nVar);VecD work2(nVar*nVar);
	       double* result(nPeriod==1?(rhs+nVar):&work1[0]);
	       double* oldJac(&work2[0]);
	       double* newJac(rhs+nVar);*/
	       
	       double t0(tFunc.evaluate());

	       for(size_t n(0);n<nPeriod;++n)
		 {
		   tFunc.set_value(t0+n);
		   for(size_t i(0);i<nVar;++i)
		     xFuncs[i].set_value(x[i]);
		   for(size_t i(0);i<nVar;++i)
		     rhs[i]=rhsFuncs[i].evaluate()-x[i];
		   for(size_t i(0);i<nVar*nVar;++i)
		     rhs[i+nVar]=jacobian[i].evaluate();
		   for(size_t i(0);i<nVar;++i)
		     rhs[nVar+i*(nVar+1)]-=1.0;
		 }
	     });
	}
      else
	if(extJac)
	  void();
      return NULL;
    }

    Newton::F* create_ode_newton_function(const EquationSystem& eqsys, bool extJac)
    {
      if(eqsys.is_internal())
	{
	  Function tFunc;
	  VecF xFuncs,rhsFuncs,funcFuncs;
	  eqsys.detach(tFunc,xFuncs,rhsFuncs,funcFuncs);
	  size_t nVar(xFuncs.size());
	  
	  VecF jacobian;
	  build_partial_derivative_matrix(xFuncs,rhsFuncs,jacobian);
	  return new Newton::F
	    ([=](const double* x, double* rhs) mutable
	     {
		   for(size_t i(0);i<nVar;++i)
		     xFuncs[i].set_value(x[i]);
		   for(size_t i(0);i<nVar;++i)
		     rhs[i]=rhsFuncs[i].evaluate();
		   for(size_t i(0);i<nVar*nVar;++i)
		     rhs[i+nVar]=jacobian[i].evaluate();
	     });
	}
      else
	{
	  if(!extJac)
	    return new Newton::F(eqsys.f());
	  else
	    {
	      F f_(eqsys.f());
	      F dfdx_(eqsys.dfdx());
	      size_t nVar(eqsys.n_variables());
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
	      return new Newton::F([f_,dfdx_,nVar](const double* x, double* rhs)
		{
		  f_(x,rhs);
		  dfdx_(x,rhs+nVar);
		});
#pragma clang diagnostic pop
	    }
	}
      return NULL;
    }

    F* create_additional_function_evaluator(const EquationSystem& eqsys)
    {
      if(eqsys.is_internal())
	{
	  Function tFunc;
	  VecF xFuncs,rhsFuncs,funcFuncs;
	  eqsys.detach(tFunc,xFuncs,rhsFuncs,funcFuncs);
	  size_t nVar(xFuncs.size());
	  size_t nFunc(funcFuncs.size());

	  return new F
	    ([=](const double* x, double* funcs) mutable
	     {
	       for(size_t i(0);i<nVar;++i)
		 xFuncs[i].set_value(x[i]);
	       for(size_t i(0);i<nFunc;++i)
		 funcs[i]=funcFuncs[i].evaluate();
	     });
	}
      else
	{
	  return new F(eqsys.func());
	}
      return NULL;
    }

    Task* create_guessing_task(std::string identifier, Log* log, Stepper* stepper, dat::Wave* varyingWave,
			       dat::Wave* evWave, double tol, size_t nPeriod, size_t showAllIterates, long secvar)
    {
      Task* task = new Task
	([=]() mutable
	 {

	   size_t nVar(stepper->nVar);	   

	   auto f = [nVar,stepper,nPeriod,tol](const double* x, double* rhs)
	   {
	     stepper->reset(x);
	     stepper->advance(nPeriod);
	     for(size_t i(0);i<nVar;++i)
	       rhs[i]=stepper->x(i)-x[i];
	     for(size_t i(0);i<nVar*nVar;++i)
	       rhs[i+nVar]=stepper->jac(i);	     
	     // subtract 1 from the diagonal of the Jacobian
	     for(size_t i(0);i<nVar;++i)
	       rhs[nVar+i*(nVar+1)]-=1.0;
	   };

	   double* x(new double[nVar]);
	   for(size_t i(0);i<nVar;++i)
	       x[i]=(*varyingWave)[uint32_t(i)+1];
	   varyingWave->remove_last_line();
	   bool success(false);
	   try
	     {
	       success=newton(int(nVar),x,f,false,tol);
	     }
	   catch(std::string error)
	     {
	       success=false;
	       log->push<Log::ERROR>(error);
	     }
	   if(!success)
	     {
	       log->push<Log::ERROR>("Newton iteration did not converge\n");
	     }
	   else
	     {
	       // get eigenvalue and eigenvector info
	       VecD evals,evecs;
	       VecSZ types;
	       double* jac(new double[nVar*nVar]);
	       for(size_t i(0);i<nVar*nVar;++i)
		 jac[i]=stepper->jac(i);
	       if(secvar>=0) // in Poincare mode, restore the artificial zero-multiplier to 1
		 jac[size_t(secvar)*(nVar+1)]=1;
	       floquet(nVar,jac,evals,evecs,types);
	       evWave->append(&evals[0],uint32_t(nVar*2));
	       evWave->append(&evecs[0],uint32_t(nVar*nVar));
	       for(size_t i(0);i<types.size();++i)
		 evWave->append(double(types[i]));
	       delete[] jac;

	       size_t factor(showAllIterates?nPeriod:1);
	       stepper->reset(x);
	       for(size_t j(0);j<factor;++j)
		 {
		   try
		     {
		       stepper->advance(nPeriod/factor);
		     }
		   catch(std::string error)
		     {
		       log->push<Log::ERROR>(error);
		       success=false;
		       break;
		     }
		   varyingWave->append(stepper->t());
		   for(size_t i(0);i<nVar;++i)
		     varyingWave->append(stepper->x(i));
		   for(size_t i(0);i<stepper->nFunc;++i)
		     varyingWave->append(stepper->func(i));
		 }
	     }
	   delete[] x;

	   if(success)
	     log->push<Log::SUCCESS>(identifier);
	   else
	     log->push<Log::FAIL>(identifier);
	 });
      return task;

    }  
					      
    
    Task* create_guessing_task(std::string identifier, Log* log, size_t nVar, size_t nFunc,
			       Newton::F* f, std::function<void(const double*,double*)>* ff,
			       dat::Wave* varyingWave, dat::Wave* evWave, bool extJac, double tol, bool mapMode)
    {
    
      int genJac(extJac?0:1);
      int map(mapMode?1:0);

      Task* task = new Task
	([nVar,nFunc,f,ff,varyingWave,evWave,tol,genJac,map,identifier,log]() mutable
	 {
	   double* x(new double[nVar]);
	   double* funcs(new double[nFunc]);
	   double t((*varyingWave)[0]);
	   for(size_t i(0);i<nVar;++i)
	     x[i]=(*varyingWave)[uint32_t(i)+1];	   

	   varyingWave->remove_last_line();
	   bool success(false);

	   try
	     {
	       success=newton(int(nVar),x,*f,genJac,tol);
	     }
	   catch(std::string error)
	     {
	       success=false;
	       log->push<Log::ERROR>(error);
	     }
	   if(!success)
	     log->push<Log::ERROR>("Newton iteration did not converge\n");
	   else
	     {
	       // get eigenvalue and eigenvector info
	       double* rhs(new double[nVar*(nVar+1)]);
	       (*f)(x,rhs);
	       double* jac=rhs+nVar;
	       VecD evals,evecs;
	       VecSZ types;
	       if(map)
		 {
		   for(size_t i(0);i<nVar;++i)
		     jac[i*(nVar+1)]+=1.0;
		   floquet(nVar,jac,evals,evecs,types);
		 }
	       else
		 {
		   eigen(nVar,jac,evals,evecs,types);
		 }

	       evWave->append(&evals[0],uint32_t(nVar*2));
	       evWave->append(&evecs[0],uint32_t(nVar*nVar));
	       for(size_t i(0);i<types.size();++i)
		 evWave->append(double(types[i]));
	       /*	       for(size_t i(0),size(size_t(evWave->data_max()));i<size;++i)
			       std::cout<<evWave->data()[i]<<std::endl;*/
	       delete[] rhs;
	     }
	   varyingWave->append(t);
	   for(size_t i(0);i<nVar;++i)
	     varyingWave->append(x[i]);
	   if(nFunc)
	     (*ff)(x,funcs);
	   for(size_t i(0);i<nFunc;++i)
	     varyingWave->append(funcs[i]);

	   delete f;
	   delete ff;
	   delete[] x;
	   delete[] funcs;

	   if(success)
	     log->push<Log::SUCCESS>(identifier);
	   else
	     log->push<Log::FAIL>(identifier);
	   
	 });
      return task;
    }
    
  } /* end namespace num */
} /* end namespace scigma */
