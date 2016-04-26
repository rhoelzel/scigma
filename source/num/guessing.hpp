#ifndef __SCIGMA_NUM_GUESSING_H__
#define __SCIGMA_NUM_GUESSING_H__

#include "../log.h"
#include "../dat/wave.h"
#include "definitions.h"
#include "newton.h"
#include "equationsystem.h"
#include "stepper.h"
#include "task.h"

namespace scigma
{
  namespace num
  {
    
    Newton::F* create_ode_newton_function(const EquationSystem& eqsys, bool extJac);

    Newton::F* create_map_newton_function(const EquationSystem& eqsys, size_t nPeriod, bool extJac);

    F* create_additional_function_evaluator(const EquationSystem& eqsys);
    
    Task* create_guessing_task(std::string identifier, Log* log, Stepper* stepper, dat::Wave* varyingWave,
			       dat::Wave* evWave, double tol, size_t nPeriod, size_t showAllIterates, long secvar);

    Task* create_guessing_task(std::string identifier, Log* log, size_t nVar, size_t nFunc,
			       Newton::F* f, std::function<void(const double*, double*)>* ff,
			       dat::Wave* varyingWave, dat::Wave* evWave, bool extJac, double tol, bool isMap);
    
  } /* end namespace num */
} /* end namespace scigma */

#endif /* __SCIGMA_NUM_GUESSING_H__ */
