#ifndef SCIGMA_NUM_INTEGRATIONSTEPPER_HPP
#define SCIGMA_NUM_INTEGRATIONSTEPPER_HPP

#include <vector>
#include "definitions.h"
#include "stepper.h"
#include "odessa.h"

namespace scigma
{
  namespace num
  {

    class IntegrationStepper:public Stepper
    {
    public:
      IntegrationStepper(const EquationSystem& eqsys, bool computeJacobian, double dt, 
			 bool stiff, double aTol, double rTol, size_t maxIter);

      virtual double t() const;
      virtual double x(size_t index) const;
      virtual double func(size_t index) const;
      virtual double jac(size_t index) const;

      virtual void reset(const double *x);

      virtual void advance(size_t n=1);
      virtual size_t n_variables() const;
      virtual size_t n_functions() const;
      
    private:
      IntegrationStepper();
      IntegrationStepper(const IntegrationStepper&);
      IntegrationStepper& operator=(const IntegrationStepper&);

      double t0_;
      double dt_;

      Function tFunc_;
      std::vector<Function> xFuncs_;
      std::vector<Function> rhsFuncs_;
      std::vector<Function> funcFuncs_;

      /*Odessa odessa_;*/
      double* x_;
      double* jac_;
    };



  } /* end namespace num */
} /* end namespace scigma */

#endif /* __SCIGMA_NUM_INTEGRATIONSTEPPER_HPP */
