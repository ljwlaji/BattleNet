#include "Config.h"
#include "Log.h"
#include <iomanip>
#include <fstream>
#include <Windows.h>


static Config* _Config = nullptr;
static mutex ConfigInstanceLoc;

Config* Config::GetInstance()
{
	if (!_Config)
	{
		ThreadLocker loc(ConfigInstanceLoc);
		if (!_Config)
			_Config = new Config();
	}
	return _Config;
}

Config::Config()
{
	char szModuleFilePath[260];
	int n = GetModuleFileNameA(0, szModuleFilePath, 260);
	szModuleFilePath[strrchr(szModuleFilePath, '\\') - szModuleFilePath + 1] = 0;
	WorkUrl = szModuleFilePath;
}

Config::~Config()
{
}

bool Config::SetSource(const char * filename)
{
	if (!filename)
		return false;
	std::string CurrentUrl = WorkUrl + "\\" + filename;
	ifstream myfile(CurrentUrl.c_str());

	if (!myfile)
		return false;

	values.clear();
	string line = "";
	while (getline(myfile, line)) // line中不包括每行的换行符
	{
		GetLineInfo(line);
	}
	myfile.close();
	sLog->OutLog("配置文件读取完毕");
	return true;
}

void Config::GetLineInfo(std::string& pstring)
{
	std::string CurrentStr = "";
	if (!pstring.empty())
	{
		CurrentStr = pstring.at(0);
		if (CurrentStr == "#")
			return;
	}
	std::string Key = "";
	for (int i = 0; i < pstring.length() + 1000; i++)
	{
		CurrentStr = "";
		if (pstring.empty())
			return;
		CurrentStr = pstring.at(0);
		if (CurrentStr == " ")
		{
			pstring.erase(0, 1);
			continue;
		}
		if (CurrentStr == "=")
		{
			pstring.erase(0, 1);
			break;
		}
		Key += CurrentStr;
		pstring.erase(0, 1);
	}
	bool StringStart = false;
	values[Key] = "";
	CurrentStr = "";
	for (int i = 0; i < pstring.length() + 1000; i++)
	{
		if (pstring.empty())
		{
			for (int i = 0; i != 1000; i++)
			{
				if (CurrentStr.empty())
					break;
				std::string TempS = "";
				TempS = CurrentStr.at(0);
				if (TempS == " ")
				{
					CurrentStr.erase(0, 1);
				}
				else break;
			}
			values[Key] = CurrentStr;
			return;
		}
		CurrentStr += pstring.at(0);
		pstring.erase(0, 1);
	}
}

bool Config::GetBoolDefault(const char* KeyName, bool Default)
{
	std::string _KeyName = KeyName;
	if (values.find(_KeyName) == values.end())
		return Default;
	else
		return !!atoi(values[KeyName].c_str());
}

int Config::GetIntDefault(const char* KeyName, int Default)
{
	std::string _KeyName = KeyName;
	if (values.find(_KeyName) == values.end())
		return Default;
	else
		return atoi(values[KeyName].c_str());
}

string Config::GetStringDefault(const char* KeyName, std::string Default)
{
	std::string _KeyName = KeyName;
	if (values.find(_KeyName) == values.end())
		return Default;
	else
		return values[KeyName];
}

float Config::GetFloatDefault(const char* KeyName, float Default)
{
	std::string _KeyName = KeyName;
	if (values.find(_KeyName) == values.end())
		return Default;
	else
		return atof(values[KeyName].c_str());
}
