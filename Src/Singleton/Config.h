#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <map>
#include <string>
#include <mutex>
using namespace std;
#define sConfig Config::GetInstance()
class Config
{
public:
	static Config* GetInstance();
	bool SetSource(const char* filename);
	bool GetBoolDefault(const char* KeyName, bool Default);
	int GetIntDefault(const char* KeyNamee, int Default);
	std::string GetStringDefault(const char* KeyName, std::string Default);
	float GetFloatDefault(const char* KeyName, float Default);
private:
	Config();
	~Config();
	void GetLineInfo(std::string& pstring);

private:
	std::string WorkUrl;
	std::map<std::string, std::string> values;
#define ThreadLocker		std::lock_guard<std::mutex>
};

#endif