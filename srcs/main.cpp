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

string quot_isinvalid(string& s, int *c)
{
	char quot = 0;
	for (size_t i = 0; i < s.size(); i++)
	{
		if (!quot && isin(s[i], "\'\"`["))
			quot = s[i];
		else if (quot && (quot == s[i] || (quot == '[' && s[i] == ']')))
			quot = 0;
		if (!quot && (s[i] == '#' && (i == 0 || std::isspace(s[i-1]))))
			s = s.substr(0, i);
	}
	if (quot)
		*c = 1;
	else
		*c = 0;
	switch (quot)
	{
	case '\'':
		return "quote";
	case '\"':
		return "dquote";
	case '`':
		return "bquote";
	case '[':
		return "error";
	default :
		return "";
	}
	return "";
}

string parsing(string &s)
{
	map<string, string> p = {{"if","fi"},{"for","done"},{"while","done"}};
	vector<string> v;
	int ifcount = 0;
	for (size_t i = 0; i < s.size(); ++i)
	{
		size_t j = i;
		if (j == 0 || s[j] == '\n' || s[j] == ';')
		{
			if (j != 0)
				++j;
			if (j == s.size())
				break;
			while (isspace(s[j]))
				++j;
			
			string token = get_first_token(s.substr(j));
			if (token == "if")
				++ifcount;
			if (token == "if" || token == "for" || token == "while")
				v.push_back(token);
			else if (token == "fi" || token == "done")
			{
				if (!v.empty() && p[v.back()] == token)
					v.pop_back();
				else
					return "error";
			}
			else if (token == "else")
			{
				--ifcount;
				if (ifcount < 0)
					return "error";
			}
		}
	}
	string prmt("");
	for (size_t i = 0; i < v.size(); ++i)
		prmt += v[i] + " ";
	return prmt;
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

int main(int argc, char** argv, char **envp)
{
	alias["ls"] = "ls --color=tty";
	alias["grep"] = "grep --color=auto";
	char *tmp = getcwd(0, 0);
	string stmp(tmp);
	free(tmp);
	jd.push_back(make_pair(stmp.substr(stmp.rfind("/") + 1), stmp));
	for (int i = 0; envp[i]; i++)
	{
		string t(envp[i]);
		int m = t.find("=");
		envm[t.substr(0, m)] = t.substr(m+1);
	}
	while (1)
	{
		char *cline = 0;
		cline = readline(makePrompt().c_str());
		if (cline == 0)
			exit(0);
		string line = cline;
		string prmt;
		int c = 0;
		while ((prmt = parsing(line) + quot_isinvalid(line, &c)) != "")
		{
			if (prmt.find("error") != string::npos)
				break;
			free(cline);
			line += c?string("\n"):string(" ; ") + (cline = readline((prmt + "> ").c_str()));
		}
		if ((line = strip(line)) != "")
			add_history(line.c_str());
		if (prmt.find("error") != string::npos)
		{
			cerr << SHELL << ": syntax error" << endl;
			continue;
		}
		command(line);
		free(cline);
	}
}
