#ifndef SCIGMA_NUM_STEPPER_HPP
#define SCIGMA_NUM_STEPPER_HPP

#include <stddef.h>

namespace scigma
{
  namespace num
  {
    class Stepper
    {
    public:
      Stepper();
      virtual ~Stepper();

      virtual double t() const =0;
      virtual double x(size_t index) const =0;
      virtual double func(size_t index) const = 0;
      virtual double jac(size_t index) const = 0;
      virtual void reset(const double* x)=0;

      virtual void advance(size_t n=1)=0;

      virtual size_t n_variables()=0;
      virtual size_t n_functions()=0;
   
    private:
      Stepper(const Stepper&);
      Stepper& operator=(const Stepper&); 
    };

  } /* end namespace num */
} /* end namespace scigma */

#endif /* __SCIGMA_NUM_STEPPER_HPP*/
