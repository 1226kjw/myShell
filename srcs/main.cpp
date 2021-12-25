#include "myshell.hpp"

map<string, pair<bool, string>> envm;
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

string makePrompt(void)
{
	string prompt = (ret==0 ? paintString("✓ ", COLOR_GREEN) : paintString("✘ ", COLOR_RED));
	if (envm.find("USER") != envm.end())
		prompt += paintString(envm["USER"].second, COLOR_GREEN);
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
		envm[t.substr(0, m)] = make_pair(true, t.substr(m+1));
	}
	if (envm.find("SHLVL") != envm.end())
		envm["SHLVL"].second = std::to_string(std::stoi(envm["SHLVL"].second)+1);
	else
		envm["SHLVL"] = make_pair(true, string("1"));
	if (argc != 1)
	{
		ifstream f;
		f.open(argv[1]);
		if (f.is_open())
		{
			envm["#"] = make_pair(false, std::to_string(argc - 2));
			for (int i = 1; i < argc; ++i)
			{
				envm[std::to_string(i-1)] = make_pair(false, string(argv[i]));
			}
			string l;
			while (!f.eof())
			{
				std::getline(f, l);
				string line = l;
				string prmt;
				int c = 0;
				while ((prmt = parsing(line) + quot_isinvalid(line, &c)) != "")
				{
					if (prmt.find("error") != string::npos)
						break;
					std::getline(f, l);
					line += c?string("\n"):string(" ; ") + l;
				}
				line = strip(line);
				if (prmt.find("error") != string::npos)
				{
					cerr << SHELL << ": syntax error" << endl;
					continue;
				}
				command(line);
			}
			f.close();
			return 0;
		}
		else
		{
			cerr << SHELL << ": " << argv[1] << ": " << strerror(errno) << endl;
			return 127;
		}
	}
	while (1)
	{
		char *cline = 0;
		cline = readline(makePrompt().c_str());
		if (cline == 0)
			exit(0);
		string line(cline);
		string prmt;
		int c = 0;
		while ((prmt = parsing(line) + quot_isinvalid(line, &c)) != "")
		{
			if (prmt.find("error") != string::npos)
				break;
			free(cline);
			line += string(c?"\n":" ; ") + (cline = readline((prmt + "> ").c_str()));
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
