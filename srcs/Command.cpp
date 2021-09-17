#include "myshell.hpp"

extern int ret;
extern map<string, string> envm;
extern map<string, string> alias;
extern vector<pair<string, string>> jd;

stringstream Command::ss;
map<string, std::function<int(vector<string>)>> Command::builtin = {
	{//cd
		"cd",
		[](vector<string> cmd)->int {
			int res = 0;
			if (cmd.size() == 1 || cmd[1] == "~")
				res = chdir(envm["HOME"].c_str());
			else
				res = chdir(cmd[1].c_str());
			if (res == 0)
			{
				char *tmp = getcwd(0, 0);
				string stmp(tmp);
				free(tmp);
				jd.push_back(make_pair(stmp.substr(stmp.rfind("/") + 1), stmp));
			}
			else
				cerr << SHELL << ": " << strerror(errno) << ": " << cmd[1] << endl;
			return res;
		}
	},
	{//export
		"export",
		[](vector<string> cmd)->int {
			if (cmd.size() == 1)
			{
				for (auto i : envm)
					cout << i.first << "=" << i.second << endl;
			}
			else
			{
				for (auto i = cmd.begin() + 1; i != cmd.end(); i++)
				{
					size_t pos = 0;
					string key = i->substr(0, pos = i->find("="));
					string value;
					if (pos != string::npos)
						value = i->substr(pos + 1);
					envm[key] = value;
				}
			}
			return 0;
		}
	},
	{//unset
		"unset",
		[](vector<string> cmd)->int {
			for (auto i = cmd.begin() + 1; i != cmd.end(); i++)
				envm.erase(*i);
			return 0;
		}
	},
	{//alias
		"alias",
		[](vector<string> cmd)->int {
			if (cmd.size() == 1)
			{
				for (auto i : alias)
					cout << i.first << "='" << i.second << "'" << endl;
			}
			else
			{
				for (auto i = cmd.begin() + 1; i != cmd.end(); i++)
				{
					size_t pos = 0;
					string key = i->substr(0, pos = i->find("="));
					string value;
					if (pos != string::npos)
						value = i->substr(pos + 1);
					if (isin(value.front(), "'\"") && value.front() == value.back())
						value = value.substr(1, value.size()-2);
					alias[key] = value;
				}
			}
			return 0;
		}
	},
	{//unalias
		"unalias",
		[](vector<string> cmd)->int {
			for (auto i = cmd.begin() + 1; i != cmd.end(); i++)
				alias.erase(*i);
			return 0;
		}
	},
	{//jd
		"jd",
		[](vector<string> cmd)->int {
			if (cmd.size() == 1)
			{
				for (auto i = jd.rbegin(); i != jd.rend(); i++)
					cout << i->first << "=" << i->second << endl;
				return 0;
			}
			auto jump = std::find_if(jd.rbegin(), jd.rend(),
						[cmd](auto i) {return i.first == cmd[1];});
			if (jd.rend() != jump)
				return chdir(jump->second.c_str());
			else
				return builtin["cd"](cmd);
		}
	}
};


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
	auto func = [this](string s)->string{
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
					s = s.substr(0, i - 1) + std::to_string(ret) + s.substr(i + 2);
					i += std::to_string(ret).size() - 1;
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
						value = envm[key];
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
	if (builtin.find(cmd_vec[0]) != builtin.end())
		return builtin[cmd_vec[0]](cmd_vec);
	else if (pid_t pid = fork() == 0)
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