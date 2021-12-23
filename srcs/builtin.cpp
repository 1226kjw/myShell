#include "myshell.hpp"

extern int ret;
extern map<string, pair<bool, string>> envm;
extern map<string, string> alias;
extern vector<pair<string, string>> jd;

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
	{
		"break",
		[](vector<string> cmd)->int {
			(void)cmd;
			return 1;
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
	}
};