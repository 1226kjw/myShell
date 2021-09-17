#include "myshell.hpp"

extern int ret;

stringstream Command::ss;
map<string, std::function<int(char **)>> Command::builtin = {
	{
		"cd",
		[](char **cmd)->int {
			int res = 0;
			if (cmd[1] == 0)
				res = chdir("~");
			else
				res = chdir(cmd[1]);
			return res;
		}
	}
};


Command::Command() {}
Command::Command(string raw, bool isfirst, bool islast): valid(1), isfirst(isfirst), islast(islast)
{
	char quot = 0;
	for (int i = 0; i < raw.size(); i++)
	{
		if (!quot && isin(raw[i], "'\""))
			quot = raw[i];
		else if (quot && raw[i] == quot)
			quot = 0;
		if (!quot && isin(raw[i], "<>"))
		{
			pair<string, int> tok;
			if (raw[i+1] == raw[i])
				tok = nextword(raw, i+2, " <>");
			else
				tok = nextword(raw, i+1, " <>");
			if (tok.first.size() == 0)
			{
				cerr << SHELL << ": syntax error near unexpected token 'newline'" << endl;
				ret = 1;
				valid = 0;
				return;
			}
			if (raw[i] == '<' && !isin(raw[i+1], "<>"))
				redin.push_back(make_pair(REDIN, tok.first));
			else if (raw[i] == '<' && raw[i+1] == '<' && !isin(raw[i+2], "<>"))
				redin.push_back(make_pair(HEREDOC, tok.first));
			else if (raw[i] == '>' && !isin(raw[i+1], "<>"))
				redout.push_back(make_pair(REDOUT, tok.first));
			else if (raw[i] == '>' && raw[i+1] == '>' && !isin(raw[i+2], "<>"))
				redout.push_back(make_pair(APPEND, tok.first));
			else
			{
				cerr << SHELL << ": syntax error near unexpected token '" << raw[i] << '\'' << endl;
				ret = 1;
				valid = 0;
				return;
			}
			for (int j = i; j < tok.second; j++)
				raw[j] = ' ';
			i = tok.second - 1;
		}
	}
	raw_command = raw;
}
Command::~Command() {}

int Command::execute(void)
{
	int stdout_fd = dup(STDOUT_FILENO);
	int status = 0;
	if (!valid)
	{
		ss = stringstream("");
		return 1;
	}
	do
	{
		out_random = "."+random_string(10);
	} while (access(out_random.c_str(), F_OK) == 0);
	do
	{
		in_random = "."+random_string(10);
	} while (access(in_random.c_str(), F_OK) == 0 || out_random == in_random);

	pid_t pid = fork();
	if (pid == 0)
	{
		for (auto i : redin)
		{
			if (i.first == REDIN)
			{
				std::ifstream file(i.second);
				if (!file.is_open())
				{
					cerr << SHELL << ": " << strerror(errno) << ": " << i.second << endl;
					exit(1);
				}
				ss << file.rdbuf();
			}
			else if (i.first == HEREDOC)
			{
				while (!cin.eof())
				{
					string line;
					getline(cin, line);
					if (line == i.second)
						break;
					ss << line << endl;
				}
			}
		}
		if (redin.size() != 0 || !isfirst)
		{
			std::ofstream inf(in_random);
			inf << ss.str();
			inf.close();
			freopen(in_random.c_str(), "r", stdin);
			remove(in_random.c_str());
			ss = stringstream("");
		}
		if (redout.size() != 0 || !islast)
		{
			std::ifstream outf(out_random);
			freopen(out_random.c_str(), "w", stdout);
		}
		char **cmd_arr = vec2cstr(split(raw_command, " "));
		if (execvp(cmd_arr[0], cmd_arr) == -1)
		{
			if (errno == 2)
			{
				cerr << SHELL << ": command not found: " << cmd_arr[0] << endl;
				deletearr(cmd_arr);
				exit(127);
			}
			else
				cerr << strerror(errno) << endl;
			deletearr(cmd_arr);
			exit(errno);
		}
	}
	else
	{
		wait(&status);
		ss = stringstream("");
		std::ifstream outf(out_random);
		ss << outf.rdbuf();
		remove(out_random.c_str());
		if (redout.size() == 0 && islast)
		{
			cout << ss.str();
			ss = stringstream("");
		}
		else
		{
			dup2(stdout_fd, STDOUT_FILENO);
			for (auto i : redout)
			{
				std::ofstream f(i.second.c_str(), ios_base::out | (i.first == APPEND ? ios_base::app : ios_base::trunc));
				f << ss.str();
			}
		}
		return WEXITSTATUS(status);
	}
}