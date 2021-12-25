#include "myshell.hpp"

extern int ret;
extern map<string, pair<bool, string>> envm;
extern map<string, string> alias;
extern vector<pair<string, string>> jd;
vector<string> dirs;

map<string, std::function<int(vector<string>)>> Command::builtin = {
	{//:
		":",
		[](vector<string> cmd)->int {
			(void)cmd;
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
	{//bg
		"bg",
		[](vector<string> cmd)->int {
			(void)cmd;
			return 0;
		}
	},
	{//break
		"break",
		[](vector<string> cmd)->int {
			(void)cmd;
			return 0;
		}
	},
	{//cd
		"cd",
		[](vector<string> cmd)->int {
			int res = 0;
			if (cmd.size() == 1)
			{
				char * tmp = getcwd(0, 0);
				envm["OLDPWD"].second = tmp;
				free(tmp);
				res = chdir(envm["HOME"].second.c_str());
			}
			else if (cmd[1] == "-")
			{
				string oldpwd = envm["OLDPWD"].second;
				char * tmp = getcwd(0, 0);
				envm["OLDPWD"].second = tmp;
				free(tmp);
				cout << oldpwd << endl;
				res = chdir(oldpwd.c_str());
			}
			else
			{
				if (cmd[1][0] == '~')
					cmd[1].replace(0, 1, envm["HOME"].second);
				char * tmp = getcwd(0, 0);
				envm["OLDPWD"].second = tmp;
				free(tmp);
				res = chdir(cmd[1].c_str());
			}
			if (res == 0)
			{
				char *tmp = getcwd(0, 0);
				string stmp(tmp);
				envm["PWD"].second = stmp;
				free(tmp);
				jd.push_back(make_pair(stmp.substr(stmp.rfind("/") + 1), stmp));
			}
			else
				cerr << SHELL << ": " << strerror(errno) << ": " << cmd[1] << endl;
			return res;
		}
	},
	{//dirs
		"dirs",
		[](vector<string>)->int {
			cout << envm["PWD"].second << ' ';
			for (auto i = dirs.rbegin(); i != dirs.rend(); ++i)
				cout << *i << ' ';
			cout << endl;
			return 0;
		}
	},
	{//exec
		"exec",
		[](vector<string> cmd)->int {
			cmd.erase(cmd.begin());
			char **cmd_arr = vec2cstr(cmd);
			if (execvp(cmd_arr[0], cmd_arr) == -1)
			{
				if (errno == 2)
					cerr << SHELL << ": command not found: " << cmd_arr[0] << endl;
				else
					cerr << strerror(errno) << endl;
			}
			return ret;
		}
	},
	{//exit
		"exit",
		[](vector<string> cmd)->int {
			if (cmd.size() == 1)
				exit(0);
			if (std::isdigit(cmd[1][0]))
				exit(stoi(cmd[1]));
			else
				exit(0);
		}
	},
	{//enable
		"enable",
		[](vector<string> cmd)->int {
			if (cmd.size() == 1)
			{
				for (auto i : builtin)
					if (i.first.find("-n") == string::npos)
						cout << "enable " << i.first << endl;
			}
			else if (cmd[1][0] == '-')
			{
				if (cmd[1][1] == 'a')
					for (auto i : builtin)
						cout << "enable " << i.first << endl;
				else if (cmd[1][1] == 'n')
				{
					builtin[string("-n ") + cmd[2]] = builtin[cmd[2]];
					builtin.erase(cmd[2]);
				}
			}
			else if (cmd.size() == 2)
			{
				if (builtin.find(string("-n ") + cmd[1]) != builtin.end())
				{
					builtin[cmd[1]] = builtin[string("-n ") + cmd[1]];
					builtin.erase(string("-n ") + cmd[1]);
				}
			}
			return 0;
		}
	},
	{//export
		"export",
		[](vector<string> cmd)->int {
			if (cmd.size() == 1)
			{
				for (auto i : envm)
					if (i.second.first)
						cout << i.first << "=" << i.second.second << endl;
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
					envm[key] = make_pair(true, value);
				}
			}
			return 0;
		}
	},
	{//popd
		"popd",
		[](vector<string>)->int {
			if (dirs.size() == 0)
			{
				cerr << SHELL << ": popd: directory stack empty" << endl;
				return 1;
			}
			vector<string> c(2);
			c[0] = "cd";
			c[1] = dirs.back();
			builtin["cd"](c);
			dirs.pop_back();
			builtin["dirs"](vector<string>(1, "dirs"));
			return 0;
		}
	},
	{//pushd
		"pushd",
		[](vector<string> cmd)->int {
			if (cmd.size() == 1)
				cmd.push_back("~");
			dirs.push_back(envm["PWD"].second);
			vector<string> c(2);
			c[0] = "cd";
			c[1] = cmd[1];
			int t = builtin["cd"](c);
			if (t == 0)
				builtin["dirs"](vector<string>(1, "dirs"));
			return t;
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