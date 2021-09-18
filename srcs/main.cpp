#include "myshell.hpp"

map<string, string> envm;
map<string, string> alias;
vector<pair<string, string>> jd;
int	ret = 0;

string git_check(void)
{
	string gitinfo = "";
	if (access(".git", F_OK) == -1)
		return "";
	return gitinfo;
}

int run_cmd(string cmd)
{
	vector<string> pipe_split = split(cmd, "|");

	Command *cmd_list = new Command[pipe_split.size()];
	for (size_t i = 0; i < pipe_split.size(); i++)
		cmd_list[i] = Command(pipe_split[i], i==0, i==pipe_split.size()-1);
	for (size_t i = 0; i < pipe_split.size(); i++)
		ret = cmd_list[i].execute();
	delete[] cmd_list;
	Command::ss = stringstream("");
	return ret;
}

string quot_isinvalid(string& s)
{
	char quot = 0;
	for (size_t i = 0; i < s.size(); i++)
	{
		if (!quot && isin(s[i], "\'\"`"))
			quot = s[i];
		else if (quot && quot == s[i])
			quot = 0;
		if (!quot && (s[i] == '#' && (i == 0 || std::isspace(s[i-1]))))
			s = s.substr(0, i);
	}
	switch (quot)
	{
	case '\'':
		return "quote> ";
	case '\"':
		return "dquote> ";
	case '`':
		return "bquote> ";
	default :
		return "";
	}
	return "";
}

string makePrompt(void)
{
	string prompt = (ret==0 ? paintString("✓ ", COLOR_GREEN) : paintString("✘ ", COLOR_RED));
	if (envm.find("USER") != envm.end())
		prompt += paintString(envm["USER"], COLOR_GREEN);
	else
		prompt += paintString(SHELL, COLOR_GREEN);
	prompt += ":";
	char *cwd = getcwd(0, 0);
	prompt += paintString(cwd, COLOR_BLUE);
	free(cwd);
	prompt += git_check();
	prompt += "$ ";
	return (prompt);
}

int main(int, char**, char **envp)
{
	alias["ls"] = "ls --color=tty";
	char *tmp = getcwd(0, 0);
	string stmp(tmp);
	free(tmp);
	jd.push_back(make_pair(stmp.substr(stmp.rfind("/") + 1), stmp));
	char *cline = 0;
	for (int i = 0; envp[i]; i++)
	{
		string t(envp[i]);
		int m = t.find("=");
		envm[t.substr(0, m)] = t.substr(m+1);
	}
	while (1)
	{
		string quot_type;
		cline = readline(makePrompt().c_str());
		if (cline == 0)
			exit(0);
		string line = cline;
		while ((quot_type = quot_isinvalid(line)) != "")
			line += string("\n") + readline(quot_type.c_str());
		if ((line = strip(line)) != "")
			add_history(cline);
		vector<string> split_next = split(line, ";");
		for (auto i: split_next)
		{
			vector<string> split_or = split(i, "||");
			for (auto j: split_or)
			{
				vector<string> split_and = split(j, "&&");
				for (auto k: split_and)
				{
					ret = run_cmd(j);
					if (ret != 0)
						break ;
				}
				if (ret == 0)
					break ;
			}
		}
		free(cline);
	}

}
