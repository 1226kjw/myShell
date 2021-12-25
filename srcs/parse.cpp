#include "myshell.hpp"

extern int ret;

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

size_t if_nextend(string s, size_t j)
{
	for (; j < s.size(); ++j)
	{
		if (j == 0 || s[j] == ';' || s[j] == '\n')
		{
			if (j != 0)
				++j;
			if (j == s.size())
				return j;
			while (isspace(s[j]))
				++j;
			string token = get_first_token(s.substr(j));
			if (token == "else" || token == "elif" || token == "fi")
				return j;
		}
	}
	return -1;
}

/*
**	if command; then;
**		command;
**	[
**	elif command; then;
**		command;
**	]
**	...
**	[
**	else
**		command;
**	]
**	fi
*/
void parse_if(string s)
{
	size_t i = 0, j = 0;
	vector<pair<int, string>> v;
	while ((i = s.find("if", j)) != string::npos)
	{
		j = s.find("then", i);
		pair<int, string> cur;
		command(s.substr(i + 2, j - i - 3));
		cur.first = ret;
		cur.second = s.substr(j + 5,if_nextend(s,j+4) -j-5);
		v.push_back(cur);
	}
	if ((i = s.find("else", j)) != string::npos)
	{
		j = s.find("then", i);
		pair<int, string> cur;
		cur.first = 0;
		cur.second = s.substr(i+5,s.rfind("fi") - i - 5);
		v.push_back(cur);
	}
	for (auto i : v)
	{
		if (!i.first)
		{
			command(i.second);
			break;
		}
	}
	int flag = 1;
	for (size_t i = 0; i < v.size(); ++i)
		flag *= v[i].first;
	if (flag)
		ret = 0;
}

size_t while_nextend(string s, size_t j)
{
	for (; j < s.size(); ++j)
	{
		if (j == 0 || s[j] == ';' || s[j] == '\n')
		{
			if (j != 0)
				++j;
			while (isspace(s[j]))
				++j;
			if (j >= s.size())
				return j;
			string token = get_first_token(s.substr(j));
			if (token == "do" || token == "done")
				return j;
		}
	}
	return -1;
}
void parse_while(string s)
{
	/*
	**	while command;
	**	do
	**		command;
	**	done
	*/
	string cond = s.substr(s.find("while") + 5, while_nextend(s, 0) - s.find("while") - 6);
	string comd = s.substr(while_nextend(s,0) + 4, s.rfind("done") - while_nextend(s,0) - 5);
	while (1)
	{
		command(cond);
		if (ret)
			break;		
		command(comd);
	}
}

void parse_for(string s)
{
	/*
	**	for command;
	**	do
	**		command;
	**	done
	*/
	// string cond = s.substr(s.find("do") + 2, while_nextend(s, 0) - s.find("do") - 3);
	// string comd = s.substr(while_nextend(s,0) + 4, s.rfind("done") - while_nextend(s,0) - 5);
	// while (1)
	// {
	// 	command(cond);
	// 	if (ret)
	// 		break;		
	// 	command(comd);
	// }
	(void)s;
}

void command(string s)
{
	size_t ii = 0, jj = s.size() - 1;
	while (ii < s.size() && isin(s[ii], " ;"))
		++ii;
	while (jj >= 0 && isin(s[jj], " ;"))
		--jj;
	if (ii > jj)
		return;
	else
		s = s.substr(ii, jj - ii + 1);
	string token = get_first_token(s);
	if (token == "")
		return;
	if (token == "if")
		parse_if(s);
	else if (token == "while")
		parse_while(s);
	else if (token == "for")
		parse_for(s);
	else
	{
		vector<string> split_next = split(s, ";");
		for (auto i: split_next)
		{
			vector<string> split_or = split(i, "||");
			for (auto j: split_or)
			{
				vector<string> split_and = split(j, "&&");
				for (auto k: split_and)
				{
					ret = run_cmd(strip(j));
					if (ret != 0)
						break ;
				}
				if (ret == 0)
					break ;
			}
		}
	}
}