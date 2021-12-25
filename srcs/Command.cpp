#include "myshell.hpp"

extern int ret;
extern map<string, pair<bool, string>> envm;
extern map<string, string> alias;
extern vector<pair<string, string>> jd;

stringstream Command::ss;

Command::Command() {}
Command::Command(string raw, bool isfirst, bool islast): valid(1), isfirst(isfirst), islast(islast)
{
	char quot = 0;
	for (size_t i = 0; i < raw.size(); i++)
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
	do {out_random = "."+random_string(10);} while (access(out_random.c_str(), F_OK) == 0);
	do {in_random = "."+random_string(10);} while (access(in_random.c_str(), F_OK) == 0 || out_random == in_random);

	vector<string> cmd_vec = split(raw_command, " ");
	vector<string> cmd_alias;
	if (alias.find(cmd_vec[0]) != alias.end())
	{
		cmd_alias = split(alias[cmd_vec[0]], " ");
		cmd_vec.erase(cmd_vec.begin());
		int pos = 0;
		for (auto i : cmd_alias)
			cmd_vec.insert(cmd_vec.begin() + pos++, i);
	}
	auto func = [this](string s)->string{//interpret env, remove outer quotes
		char quot = 0;
		for (size_t i = 0; i < s.size(); i++)
		{
			if (!quot && isin(s[i], "'\""))
			{
				quot = s[i];
				s = s.substr(0, i) + s.substr(i+1);
				i--;
			}
			else if (quot && s[i] == quot)
			{
				s = s.substr(0, i) + s.substr(i+1);
				quot = 0;
				i--;
			}
			else if (s[i] == '$' && quot != '\'')
			{
				if (s[i+1] == '?')
				{
					s = s.substr(0, i) + std::to_string(ret) + s.substr(i + 2);
					i += std::to_string(ret).size() - 1;
				}
				else if (s[i+1] == '$')
				{
					s = s.substr(0, i) + std::to_string(getpid()) + s.substr(i + 2);
					i += std::to_string(getpid()).size() - 1;
				}
				else if (s[i+1] == '#')
				{
					s = s.substr(0, i) + envm["#"].second + s.substr(i + 2);
					i += envm["#"].second.size() - 1;
				}
				else
				{
					int j = ++i;
					bool flag = false;
					if (s[i] == '{')
					{
						flag = true;
						i++;
					}
					while (std::isalnum(s[i]) || s[i] == '_')
						i++;
					string key;
					if (flag)
					{
						if (s[i] == '}')
						{
							key = s.substr(j + 1, i - j - 1);
							i++;
						}
						else
						{
							cerr << SHELL << ": bad substitution" << endl;
							ret = 1;
							this->valid = 0;
						}
					}
					else
						key = s.substr(j, i - j);
					string value;
					if (key.size() > 0 && envm.find(key) != envm.end())
						value = envm[key].second;
					else if (key.size() > 0)
						value = "";
					else
						value = "$";
					s = s.substr(0, j-1) + value + s.substr(i);
					i += value.size() - key.size() - 2;
					if (flag)
						i -= 2;
				}
			}
		}
		return s;
	};
	std::transform(redin.begin(), redin.end(), redin.begin(), [func](auto i){return make_pair(i.first, func(i.second));});
	std::transform(redout.begin(), redout.end(), redout.begin(), [func](auto i){return make_pair(i.first, func(i.second));});
	std::transform(cmd_vec.begin(), cmd_vec.end(), cmd_vec.begin(), [func](auto i){return func(i);});
	bool stdflag = false;
	if (cmd_vec[0] == "stdout")
	{
		stdflag = true;
		cmd_vec.erase(cmd_vec.begin());
	}
	else if (cmd_vec[0] == "eval")
		cmd_vec.erase(cmd_vec.begin());
	if (builtin.find(cmd_vec[0]) != builtin.end())
		return builtin[cmd_vec[0]](cmd_vec);
	else if (std::count(cmd_vec[0].begin(), cmd_vec[0].end(), '=') == 1 && cmd_vec[0][0] != '=')
	{
		vector<string> keyval = split(cmd_vec[0], "=");
		if (keyval.size() == 1)
			keyval.push_back("");
		if (envm.find(keyval[0]) != envm.end())
			envm[keyval[0]].second = keyval[1];
		else
			envm[keyval[0]] = make_pair(false, keyval[1]);
		return 0;
	}
	else if (fork() == 0)
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
					cout << i.second << "> ";
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
		char **cmd_arr = vec2cstr(cmd_vec);
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
			if (stdflag)
				cout << ss.str();
			dup2(stdout_fd, STDOUT_FILENO);
			for (auto i : redout)
			{
				std::ofstream f(i.second.c_str(), ios_base::out | (i.first == APPEND ? ios_base::app : ios_base::trunc));
				f << ss.str();
			}
		}
		return WEXITSTATUS(status);
	}
	return -1;
}