#include <cmath>
#include <cstdlib>
#include <cctype>
#include <catch.hpp>
#include "../externalequationsystem.hpp"

#define REQUIRE_EXCEPTION(X,Y) {std::string error;try{X;}catch(std::string err){error=err;}REQUIRE(error==Y+error.substr(std::string(Y).size()));}

using namespace scigma::num;

extern const std::string allowed;

extern std::string well_formed_name(int maxLength);
extern std::string ill_formed_name(int maxLength);

void f_t(double t, const double* x, double* rhs)
{
  rhs[0]=t*(x[0]+x[1]*x[0]);
  rhs[1]=(x[0]*x[1]-x[1])/t;
}

void dfdx_t(double t, const double* x, double* dfdx)
{
  dfdx[0]=t*(1+x[1]);dfdx[2]=t*x[0];
  dfdx[1]=x[1]/t;dfdx[3]=(x[0]-1)/t;
}

void func_t(double t, const double* x, double* funcval)
{
  funcval[0]=std::sin(x[0]*t+x[1]);
  funcval[1]=std::cos(x[0]+x[1])-t;
  funcval[2]=std::tan(x[0]+x[1]*t);
}

SCENARIO ("non-autonomous external ODEs without parameters","[equationsystem][external]")
{
  std::srand(std::time(NULL));

  GIVEN("a vector of variable names (varNames), the right hand side (f) of the ODE [optional: a Jacobian (dfdx_t), a set of function names (funcNames), the associated evaluation routine (func_t)]")
    {
      VecS varNames;
      varNames.push_back("x");
      varNames.push_back("y");
      
      VecS funcNames;
      funcNames.push_back("f_1");
      funcNames.push_back("f_2");
      funcNames.push_back("f_3");

      WHEN("varNames is empty")
	THEN("exception \"need at least one variable\" is thrown")
	REQUIRE_EXCEPTION((ExternalEquationSystem(VecS(),f_t)),"need at least one variable");

      THEN("exception \"right hand side cannot be NULL\" is thrown")
	{
	  F nullfunc(NULL);
	  REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,nullfunc)),"right hand side cannot be NULL");
	}

      WHEN("varNames and funcNames are well-formed")
	{
	  THEN("no exception \"ill-formed <type> name: <name>\" is thrown")
	    {
	      for(int i(0);i<100;++i)
		{
		  varNames[1]=well_formed_name(10);
		  CAPTURE(varNames[1]);
		  REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,f_t)),"");
		}
	      for(int i(0);i<100;++i)
		{
		  funcNames[0]=well_formed_name(10);
		  CAPTURE(funcNames[0]);
		  REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,f_t,NULL, funcNames, func_t)),"");
		}
	    }
	}
      
      WHEN("any name in varNames or funcNames is ill-formed")
	{
	  THEN("exception \"ill-formed <type> name: <name>\" is thrown")
	    {
	      for(int i(0);i<100;++i)
		{
		  varNames[1]=ill_formed_name(10);
		  CAPTURE(varNames[1]);
		  REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,f_t)),"ill-formed variable name:");
		}
	      varNames[1]="y";
	      for(int i(0);i<100;++i)
		{
		  funcNames[0]=ill_formed_name(10);
		  CAPTURE(funcNames[0]);
		  REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,f_t,NULL, funcNames, func_t)),"ill-formed function name:");
		}
	    }
	}
      WHEN("names in varNames and/or funcNames are not unique")
	{
	  THEN("exception \"name is not unique: <name>\" is thrown")
	    {
	      varNames[1]="x";
	      REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,f_t)),"name is not unique: x");
	      varNames[1]="y";
	      funcNames[2]="f_2";
	      REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,f_t,NULL,funcNames,func_t)),"name is not unique: f_2");
	      funcNames[2]="x";
	      REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,f_t,NULL,funcNames,func_t)),"name is not unique: x");
	    }
	}

      WHEN("EquationSystem is constructed with varNames and f_t only")

      WHEN("funcNames is non-empty and func_t is NULL")
	THEN("exception \"provided function names but no evaluation function\" is thrown")
	REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,f_t,NULL,funcNames)),"provided function names but no evaluation function");

      WHEN("vector funcNames is empty and func_t is not NULL")
	THEN("exception \"provided evaluation function but no function names\" is thrown")
	REQUIRE_EXCEPTION((ExternalEquationSystem(varNames,f_t,NULL,VecS(),func_t)),"provided evaluation function but no function names");

      WHEN("the ExternalEquationSystem has been successfully constructed with just varNames and f_t")
	{
	  ExternalEquationSystem eqsys(varNames,f_t);
	  double t(0.5);
	  eqsys.set("t",0.5);
	  double x[]={1.2345,6.7890};
	  eqsys.set("x",1.2345); eqsys.set("y",6.7890);
	  double rhs_1[2]; double rhs_2[2];
	  f_t(t,x,rhs_1);

	  THEN("dfdx(), dfdx_p(), dfdx_t()  and  dfdx_pt() return NULL")
	    {
	      REQUIRE(!eqsys.dfdx());
	      REQUIRE(!eqsys.dfdx_p());
	      REQUIRE(!eqsys.dfdx_t());
	      REQUIRE(!eqsys.dfdx_pt());
	    }
	  THEN("dfdp_p() and dfdp_pt() return NULL")
	    {
	      REQUIRE(!eqsys.dfdp_p());
	      REQUIRE(!eqsys.dfdp_pt());
	    }
	  THEN("func(), func_p(), func_t() and  func_pt() return NULL")
	    {
	      REQUIRE(!eqsys.func());
	      REQUIRE(!eqsys.func_p());
	      REQUIRE(!eqsys.func_t());
	      REQUIRE(!eqsys.func_pt());
	    }
	  THEN("is_autonomous() returns false")
	    REQUIRE(!eqsys.is_autonomous());
	  THEN("n_variables() returns the correct number")
	    REQUIRE(eqsys.n_variables()==2);
	  THEN("n_parameters(), n_functions(), n_constants() return 0")
	    {
	      REQUIRE(eqsys.n_parameters()==0);
	      REQUIRE(eqsys.n_functions()==0);
	      REQUIRE(eqsys.n_constants()==0);
	    }
	  THEN("variable_names() returns the correct names")
	    {
	      REQUIRE(eqsys.variable_names()[0]=="x");
	      REQUIRE(eqsys.variable_names()[1]=="y");
	    }
	  THEN("setting and retrieving t by name works")
	    {
	      eqsys.set("t",-2);
	      REQUIRE(eqsys.get("t")==-2);
	    }
	  THEN("time() returns the correct value")
	    {
	      eqsys.set("t",-2);
	      REQUIRE(eqsys.time()==-2);
	    }
	  THEN("setting and retrieving variables by name works")
	    {
	      eqsys.set("x",9);
	      eqsys.set("y",0.4);
	      REQUIRE(eqsys.get("x")==9);
	      REQUIRE(eqsys.get("y")==0.4);
	    }
	  AND_THEN("variable_values() returns the correct values")
	    {
	      REQUIRE(eqsys.variable_values()[0]==1.2345);
	      REQUIRE(eqsys.variable_values()[1]==6.7890);
	    }
	  THEN("f() returns the wrapper of the original right hand side function f_t for the current time")
	    {
	      eqsys.f()(x,rhs_2);
	      REQUIRE(rhs_1[0]==rhs_2[0]);
	      REQUIRE(rhs_1[1]==rhs_2[1]);
	    }
	  THEN("f_p() returns a wrapper of the original right hand side function f_t for the current time with the additional parameter p")
	    {
	      eqsys.f_p()(x,NULL,rhs_2);
	      REQUIRE(rhs_1[0]==rhs_2[0]);
	      REQUIRE(rhs_1[1]==rhs_2[1]);
	    }
	  THEN("f_t() returns the original right hand side function f_t")
	    {
	      eqsys.f_t()(t,x,rhs_2);
	      REQUIRE(rhs_1[0]==rhs_2[0]);
	      REQUIRE(rhs_1[1]==rhs_2[1]);
	    }
	  THEN("f_pt() returns a wrapper of the original right hand side function f_t with the additional parameter p")
	    {
	      eqsys.f_pt()(t,x,NULL,rhs_2);
	      REQUIRE(rhs_1[0]==rhs_2[0]);
	      REQUIRE(rhs_1[1]==rhs_2[1]);
	    }
	}
      WHEN("the Jacobian (dfdx_t) has been provided")
	{
	  ExternalEquationSystem eqsys(varNames,f_t,dfdx_t);
	  double t(0.5);
	  eqsys.set("t",0.5);
	  double x[]={5.4321,0.9876};
	  double jac_1[4]; double jac_2[4];
	  dfdx_t(t,x,jac_1);
	  
	  THEN("dfdx() returns a wrapper of the original Jacobian dfdx_t for the current time")
	    {
	      eqsys.dfdx()(x,jac_2);
	      REQUIRE(jac_1[0]==jac_2[0]);
	      REQUIRE(jac_1[1]==jac_2[1]);
	      REQUIRE(jac_1[2]==jac_2[2]);
	      REQUIRE(jac_1[3]==jac_2[3]);
	    }
	  THEN("dfdx_p() returns a wrapper of the original Jacobian dfdx_t for the current time with the additional parameter p")
	    {
	      eqsys.dfdx_p()(x,NULL,jac_2);
	      REQUIRE(jac_1[0]==jac_2[0]);
	      REQUIRE(jac_1[1]==jac_2[1]);
	      REQUIRE(jac_1[2]==jac_2[2]);
	      REQUIRE(jac_1[3]==jac_2[3]);
	    }
	  THEN("dfdx_t() returns a wrapper of the original Jacobian dfdx with the additional parameter t")
	    {
	      eqsys.dfdx_t()(t,x,jac_2);
	      REQUIRE(jac_1[0]==jac_2[0]);
	      REQUIRE(jac_1[1]==jac_2[1]);
	      REQUIRE(jac_1[2]==jac_2[2]);
	      REQUIRE(jac_1[3]==jac_2[3]);
	    }
	  THEN("dfdx_pt() returns a wrapper of the original Jacobian dfdx_t with the additional parametr p")
	    {
	      eqsys.dfdx_pt()(t,x,NULL,jac_2);
	      REQUIRE(jac_1[0]==jac_2[0]);
	      REQUIRE(jac_1[1]==jac_2[1]);
	      REQUIRE(jac_1[2]==jac_2[2]);
	      REQUIRE(jac_1[3]==jac_2[3]);
	    }
	}
      WHEN("additional functions (funcNames, func_t) have been provided")
	{
	  ExternalEquationSystem eqsys(varNames,f_t,dfdx_t,funcNames,func_t);
	  double t(0.5);
	  eqsys.set("t",0.5);
	  double x[]={9.8765,4.3210};
	  eqsys.set("x",9.8765); eqsys.set("y",4.3210);
	  double funcvals_1[3]; double funcvals_2[3];
	  func_t(t,x,funcvals_1);
	  
	  THEN("n_functions() returns the correct number")
	    REQUIRE(eqsys.n_functions()==3);
	  THEN("function_names() returns the correct names")
	    {
	      REQUIRE(eqsys.function_names()[0]=="f_1");
	      REQUIRE(eqsys.function_names()[1]=="f_2");
	      REQUIRE(eqsys.function_names()[2]=="f_3");
	    }
	  THEN("retrieving functions by name works")
	    {
	      REQUIRE(eqsys.get("f_1")==std::sin(9.8765*t+4.3210));
	      REQUIRE(eqsys.get("f_2")==std::cos(9.8765+4.3210)-t);
	      REQUIRE(eqsys.get("f_3")==std::tan(9.8765+4.3210*t));
	    }
	  THEN("function_values() returns the correct values")
	    {
	      REQUIRE(eqsys.function_values()[0]==std::sin(9.8765*t+4.3210));
	      REQUIRE(eqsys.function_values()[1]==std::cos(9.8765+4.3210)-t);
	      REQUIRE(eqsys.function_values()[2]==std::tan(9.8765+4.3210*t));
	    }
	  THEN("func() returns a wrapper of the original function func_t for the current time")
	    {
	      eqsys.func()(x,funcvals_2);
	      REQUIRE(funcvals_1[0]==funcvals_2[0]);
	      REQUIRE(funcvals_1[1]==funcvals_2[1]);
	      REQUIRE(funcvals_1[2]==funcvals_2[2]);
	    }
	  THEN("func_p() returns a wrapper of the original function func_t for the current time with the additional parameter p")
	    {
	      eqsys.func_p()(x,NULL,funcvals_2);
	      REQUIRE(funcvals_1[0]==funcvals_2[0]);
	      REQUIRE(funcvals_1[1]==funcvals_2[1]);
	      REQUIRE(funcvals_1[2]==funcvals_2[2]);
	    }
	  THEN("func_t() returns the original function func_t")
	    {
	      eqsys.func_t()(t,x,funcvals_2);
	      REQUIRE(funcvals_1[0]==funcvals_2[0]);
	      REQUIRE(funcvals_1[1]==funcvals_2[1]);
	      REQUIRE(funcvals_1[2]==funcvals_2[2]);
	    }
	  THEN("func_pt() returns a wrapper of the original function func_t with the additional parameter p")
	    {
	      eqsys.func_pt()(t,x,NULL,funcvals_2);
	      REQUIRE(funcvals_1[0]==funcvals_2[0]);
	      REQUIRE(funcvals_1[1]==funcvals_2[1]);
	      REQUIRE(funcvals_1[2]==funcvals_2[2]);
	    }
	}
    }
}
