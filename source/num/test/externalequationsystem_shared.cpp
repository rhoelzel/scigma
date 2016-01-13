#include <string>
#include <cctype>
#include <cstdlib>

const std::string allowed("ABCDEFGHIJKLMNOPQRTSUVWXYZabcdefghijklmnopqrstuvwxyz_1234567890");

std::string well_formed_name(int maxLength)
{
  int length(std::rand()%(maxLength-1)+1);
  std::string result;
  for(int i(0);i<length;++i)
    {
      result+=allowed[std::rand()%allowed.size()];
      if(i==0&&std::isdigit(result[0]))
	{
	  result="";
	  --i;
	}
    }
  if(result=="t")
    return "s";
  return result;
}

std::string ill_formed_name(int maxLength)
{
  std::string result(well_formed_name(maxLength));
  char c;
  c = std::rand()%256;
  if(std::isdigit(c))
    {
      result[0]=c;
    }
  else
    {
      if((!std::isalnum(c))&&c!='_'&&c!='\n'&&c!='\r'&&c!='\a'&&c!='\f'&&c!='\v')
	{
	  int pos = std::rand()%result.size();
	  result[pos]=c;
	}
      else
	{
	  result[0]=' ';
	}
    }
  return result;
}
