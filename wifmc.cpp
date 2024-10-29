#include<map>
#include<string>
#include<algorithm>
#include<list>
#include<vector>
#include<utility>
#include<fstream>
#include<sstream>
#include<string.h>
#include<stdio.h>
#include <sys/stat.h>

bool does_file_exist(const std::string& filename)
{
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}
std::string get_fish_history_path()
{
  std::string home = std::getenv("HOME");
  if(home.empty())
    {
      printf("%s\n","wimfc error: Failed to obtain home directory path!");
      exit(-1);
    }
  home+="/.local/share/fish/fish_history";
  
  if(!does_file_exist(home))
    {
      printf("%s\n","wimfc error: Fish history does not exist!");
      exit(-1);
    }
  return home;
}

std::list<std::string> split_string(const std::string& str, char delimiter)
{
  std::list<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(str);

  while (std::getline(tokenStream, token, delimiter))
    {
      tokens.push_back(token);
    }

  return tokens;
}
std::list<std::string> read_history(const std::string& history_path)
{
  std::list<std::string> history;
  std::ifstream hist_file(history_path);
  if(!hist_file.is_open())
    {
     printf("%s\n","wimfc error: Failed to open history file!");
      exit(-1); 
    }

  std::string line;
  while(std::getline(hist_file,line))
    {
      if(line.find("cmd") != std::string::npos)
	{
	  line = line.substr(7);
	  line = line.substr(0,line.find(' '));
	  history.push_back(line);
	}
    }
  return history;
}

typedef std::map<std::string, long long int> counter_t;
void set_element(std::string& command, counter_t& counter)
{
  if(counter.find(command) == counter.end())
    {
      counter[command] = 1;
    }
  else
    {
      ++counter[command];
    }
}
void process_sudo(std::list<std::string>& line, counter_t& counter)
{
  if(line.size() == 1) return;
  set_element(*(++line.begin()),counter);
}

void process_pipe(const std::string& line, counter_t& counter)
{
  auto splitted = split_string(line,'|');
  for(auto& el: splitted)
    {
      auto words = split_string(el,' ');
      if(el.find("sudo") == std::string::npos)
	{
	  set_element(*words.begin(),counter);
	}
      else
	{
	  set_element(*(++words.begin()),counter);
	}
    }
}

struct Arguments
{
  bool show_frequency;
  int top;
  bool show_help;
  Arguments(bool sf=false,bool sh=false,int top=1):show_frequency(sf),top(top),show_help(sh)
  {
  }
  ~Arguments(){}
};
bool is_digit_str(const std::string& str)
{
  for(auto& ch: str)
    {
      if(!isdigit(ch))return false;
    }
  return true;
}
Arguments* parse_arguments(int argc, char** argv)
{
  auto args = new Arguments();
  for(int i = 1;i<argc;++i)
    {
      std::string arg = argv[i];
      if(arg == "-h" || arg == "--help")
	{
	  args->show_help=true;
	  continue;
	}

      auto top1 = arg.find("-c=");
      auto top2 = arg.find("--top=");
      auto freq1 = arg.find("-s");
      auto freq2 = arg.find("--show-frequency");
      auto nofound = std::string::npos;
      if(top1 != nofound)
	{
	  auto val = arg.substr(3);
	  if(!is_digit_str(val))
	    {
	      printf("%s %s\n","wimfc error: Unknown argument",arg.c_str());
	      exit(-1);
	    }
	  args->top = atoi(val.c_str());
	  continue;
	}
      else if(top2 != nofound)
	{
	  auto val = arg.substr(6);
	  if(!is_digit_str(val))
	    {
	      printf("%s %s\n","wimfc error: Unknown argument",arg.c_str());
	      exit(-1);
	    }
	  args->top = atoi(val.c_str());
	  continue;
	}
      else if(freq1 != nofound)
	{
	  args->show_frequency = true;
	  continue;
	}
      else if(freq2 != nofound)
	{
	  args->show_frequency = true;
	  continue;
	}
      else
	{
	  printf("%s %s\n","wimfc error: Unknown argument",arg.c_str());
	  exit(-1);
	}
    }

  return args;
}

int main(int argc, char** argv)
{
  auto args = parse_arguments(argc,argv);
  
  counter_t counter;
  
  auto history = read_history(get_fish_history_path());
  for(auto& e: history)
    {
      auto words = split_string(e,' ');
      if(e.find("sudo") != std::string::npos)
	{
	  process_sudo(words,counter);
	}
      else if(e.find('|') != std::string::npos)
	{
	  process_pipe(e,counter);
	}
      else
	{
	  set_element(*words.begin(),counter);
	}
    }

  std::vector<std::pair<std::string,long long int>> data;
   for(auto& el: counter)data.push_back(el);

    std::sort(data.begin(),data.end(),[&](std::pair<std::string,long long int> a,
  				std::pair<std::string,long long int> b)
  				  {
  				    return a.second > b.second;
  				  });

  for(int i = 0;i<args->top;++i)
    {
      printf("%s ",data[i].first.c_str());
      if(args->show_frequency)
	printf("%d",(int)data[i].second);
      printf("\n");
    }
  
  return 0;
}
