#include "myshell.hpp"

string strip(string s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c){return !std::isspace(c);}));
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int c){return !std::isspace(c);}).base(), s.end());
	return s;
}

string paintString(string s, string color)
{
	return color + s + COLOR_RESET;
}

char **vec2cstr(vector<string> s)
{
	char **arr = new char*[s.size() + 1];
	for (unsigned int i = 0; i < s.size(); i++)
		arr[i] = strdup(s[i].c_str());
	arr[s.size()] = 0;
	return arr;
}

void deletearr(char **arr)
{
	for (int i = 0; arr[i]; i++)
		free(arr[i]);
	delete[] arr;
}

vector<string> split(string line, string divide)
{
	if (line == "")
		return vector<string>();
	if (divide.size() == 0)
	{
		vector<string> v(1, string(line));
		return v;
	}
	unsigned int i,j;
	char quot = 0;
	vector<string> v;
	for (i=0,j=0; j < line.size();)
	{
		if (!quot && line.substr(j, divide.size()) == divide)
		{
			if (j - i > 0)
				v.push_back(line.substr(i, j - i));
			j += divide.size();
			i = j;
		}
		else
			j++;
		if (!quot && (line[j] == '\'' || line[j] == '\"'))
			quot = line[j];
		else if (quot && line[j] == quot)
			quot = 0;
	}
	if (i < line.size())
		v.push_back(line.substr(i));
	return v;
}

pair<string, int> nextword(string &line, int start, string divide)
{
	unsigned int i,j;
	char quot = 0;
	for (i = start,j = start; j < line.size();)
	{
		if (!quot && isin(line[j], divide))
		{
			if (j - i > 0)
				break;
			j += 1;
			i = j;
		}
		else
			j++;
		if (!quot && (line[j] == '\'' || line[j] == '\"'))
			quot = line[j];
		else if (quot && line[j] == quot)
			quot = 0;
	}
	return make_pair(line.substr(i, j-i), j);
}

string random_string(int len)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dis(0, 10 * 26 * 3 - 1);
	stringstream ss;
	for (int i = 0; i < len; i++)
	{
		int num = dis(gen);
		switch (num % 3)
		{
		case 0:
			ss << static_cast<char>('0' + num % 10);
			break;
		case 1:
			ss << static_cast<char>('a' + num % 26);
			break;
		case 2:
			ss << static_cast<char>('A' + num % 26);
			break;
		default:
			ss << '_';
			break;
		}
	}
	return ss.str();
}

bool isin(char c, string s)
{
	return s.find(c) != string::npos;
}

bool isin(string c, string s)
{
	return s.find(c) != string::npos;
}

string get_first_token(string s)
{
	size_t i = 0;
	while (i < s.size() && isin(s[i], " ;"))
		++i;
	s = s.substr(i);
	vector<string> t = split(s, " ");
	if (t.size() == 0)
		return "";
	vector<string> t2 = split(t[0], ";");
	if (t2.size() == 0)
		return "";
	return t2[0];
}