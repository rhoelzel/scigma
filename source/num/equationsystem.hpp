#ifndef SCIGMA_NUM_EQUATIONSYSTEM_HPP
#define SCIGMA_NUM_EQUATIONSYSTEM_HPP

#include <string>
#include <functional>

typedef std::function<void(const double*, double*)> F;
typedef std::function<void(const double*, const double*, double*)> F_p;
typedef std::function<void(double t, const double*, double*)> F_t;
typedef std::function<void(double t, const double*, const double*, double*)> F_pt;

namespace scigma
{
  namespace num
  {

    class EquationSystem
    {
      
    public:
      EquationSystem();
      virtual ~EquationSystem();
      
      virtual void set(const std::string& name, double value)=0;
      virtual double get(const std::string& name)=0;
      
      virtual size_t n_variables() const=0;
      virtual size_t n_parameters() const=0;
      virtual size_t n_functions() const=0;
      virtual size_t n_constants() const=0;

      virtual const double& time() const=0;
      virtual const double* variable_values() const=0;
      virtual const double* parameter_values() const=0;
      virtual const double* function_values() =0;
      virtual const double* constant_values() =0;

      virtual const std::string* variable_names() const=0;
      virtual const std::string* parameter_names() const=0;
      virtual const std::string* function_names() const=0;
      virtual const std::string* constant_names() const=0;

      virtual bool is_autonomous() const=0;

      virtual F f() const=0;
      virtual F dfdx() const=0;
      virtual F func() const=0;
      virtual F_p f_p() const=0;
      virtual F_p dfdx_p() const=0;
      virtual F_p dfdp_p() const=0;
      virtual F_p func_p() const=0;
      virtual F_t f_t() const=0;
      virtual F_t dfdx_t() const=0;
      virtual F_t func_t() const=0;
      virtual F_pt f_pt() const=0;
      virtual F_pt dfdx_pt() const=0;
      virtual F_pt dfdp_pt() const=0;
      virtual F_pt func_pt() const=0;

    private:
      EquationSystem(const EquationSystem&);
      EquationSystem& operator=(const EquationSystem&); 

    };
    
  } /* end namespace num */
} /* end namespace scigma */

#endif /* SCIGMA_NUM_EQUATIONSYSTEM_HPP */

