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

string quot_isinvalid(string s)
{
	char quot = 0;
	for (size_t i = 0; i < s.size(); i++)
	{
		if (!quot && isin(s[i], "\'\""))
			quot = s[i];
		else if (quot && quot == s[i])
			quot = 0;
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
		string line;
		char *cwd = getcwd(0, 0);
		cline = readline(((ret==0?paintString("✓", COLOR_GREEN):paintString("✘", COLOR_RED))
							+ paintString(cwd, COLOR_BLUE) + git_check() + "$ ").c_str());
		if (cline == 0 || (line = cline) == "exit")
		{
			free(cline);
			free(cwd);
			exit(0);
		}
		while ((quot_type = quot_isinvalid(line)) != "")
			line += string("\n") + readline(quot_type.c_str());
		if ((line = strip(line)) != "")
			add_history(cline);
		vector<string> v = split(line, ";");
		for (auto i: v)
		{
			vector<string> w = split(i, "&&");
			for (auto j: w)
			{
				ret = run_cmd(j);
				if (ret != 0)
					break ;
			}
		}
		free(cline);
		free(cwd);
	}

}
