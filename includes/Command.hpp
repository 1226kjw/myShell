#ifndef COMMAND_HPP
# define COMMAND_HPP

# include "myshell.hpp"

# define REDIN 0
# define HEREDOC 1
# define REDOUT 2
# define APPEND 3

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::pair;
using std::map;
using std::find;
using std::stringstream;
using std::ios_base;
using std::cerr;

class Command
{
public:
	int valid;
	bool isfirst;
	bool islast;
	string raw_command;
	vector<pair<int, string>> redin;	// <, <<
	vector<pair<int, string>> redout;	// >, >>
	static map<string, std::function<int(char **)>> builtin;
	string out_random;
	string in_random;
	static stringstream ss;
	Command();
	Command(string raw, bool isfirst, bool islast);
	~Command();
	int execute();
};

#endif
