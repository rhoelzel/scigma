#include <string>

const std::string lorenz[] = {
  "x'=s*(y-x)\n",
  "y'=x*(r-z)-y\n",
  "z'=x*y-b*z\n",
  "r=28.0\n",
  "s=10.0\n",
  "b=2.66666667\n",
  "f_1=x**2-y**2\n",
  "f_2=sqrt(abs(f_1+z))\n",
  "c=r+s*sin(b)\n",
  "x=-1.24\n",
  "y=$c*12\n",
  "z=-tan(23)\n",
};
const size_t lorenzLines(12);
