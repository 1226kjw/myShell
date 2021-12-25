#ifndef MYSHELL_HPP
# define MYSHELL_HPP

# include <iostream>
# include <fstream>
# include <sstream>
# include <string>
# include <vector>
# include <map>
# include <set>
# include <utility>
# include <algorithm>
# include <unistd.h>
# include <fcntl.h>
# include <cstring>
# include <cerrno>
# include <random>
# include <regex>
# include <sys/wait.h>
# include <sys/stat.h>
# include <readline/readline.h>
# include <readline/history.h>
# include "Command.hpp"
# include "parse.hpp"

# define COLOR_BLUE "\033[1;34m"
# define COLOR_RED "\033[1;31m"
# define COLOR_GREEN "\033[1;32m"
# define COLOR_RESET "\033[0m"
# define SHELL "myShell"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::map;
using std::set;
using std::stringstream;
using std::cerr;
using std::ifstream;

string quot_isinvalid(string& s, int *c);
string parsing(string &s);
int run_cmd(string cmd);
string strip(string s);
string paintString(string s, string color);
char **vec2cstr(vector<string> s);
void deletearr(char **arr);
vector<string> split(string line, string divide);
pair<string, int> nextword(string &line, int start, string divide);
string random_string(int len);
bool isin(char c, string s);
bool isin(string c, string s);
string get_first_token(string s);
std::string regex_replace(const string& input, const std::regex& regex, 
		std::function<string(std::smatch const&)> format);

#endif
