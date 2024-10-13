#include<unordered_map>
#include<string>
#include<algorithm>
#include<utility>
#include<stdio.h>
#include<stdlib.h>
#include <sys/stat.h>

std::string get_home()
{
  auto home = std::getenv("HOME");
  if(home == nullptr) return "";

  return home;
}
bool does_file_exist(const std::string& filename)
{
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}
int main()
{
  auto home = get_home();
  if(home.empty())
    {
      printf("%s\n","wimfc error: Failed to obtain home directory path!");
      return -1;
    }
  home+="/.local/share/fish/fish_history";
  
  if(!does_file_exist(home))
    {
      printf("%s\n","wimfc error: Fish history does not exist!");
      return -1;
    }

  FILE* hist = fopen(home.c_str(),"r");
  if(!hist)
    {
      printf("%s%s %s\n","wimfc error: Failed to open ",home.c_str()," file!");
      return -1;
    }

  char line[256];
  std::unordered_map<std::string,long long int> counter;
  while (fgets(line, sizeof(line), hist) != NULL)
    {
      auto l = std::string(line);
      if(l.find("cmd") != std::string::npos)
	{
	  l = l.substr(7);
	  l = l.substr(0,l.find(' '));
	  if(counter.find(l) != counter.end())
	    {
	      ++counter[l];
	    }
	  else
	    {
	      counter[l] = 1;
	    }
	}    
  }
  fclose(hist);
  
  auto m = *std::max_element(counter.begin(),counter.end(),
			     [&](std::pair<std::string,long long int> a,
				std::pair<std::string,long long int> b)
				  {
				    return a.second < b.second;
				  });

  printf("%s\n",m.first.c_str());
  return 0;
}
