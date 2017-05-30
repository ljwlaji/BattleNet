#ifndef __LOG_H__
#define __LOG_H__

#include <mutex>
#include <list>
using namespace std;
#define sLog Log::GetInstance()

enum LogColor
{
	Cmd_Normal,
	Cmd_White,
	Cmd_Red,
	Cmd_Green,
	Cmd_Blue,
	Cmd_Yellow,
	Cmd_Pink,
};

class Log
{
public:
	static Log* GetInstance();
	void OutBug(const char* args, ...);
	void OutLog(const char* args, ...);
	void OutWarning(const char* args, ...);
	void OutSuccess(const char* args, ...);
	void OnStart();
	void SetColor(LogColor _color);
	void ReSetColor();
private:
	Log();
	~Log();
#define ThreadLocker		std::lock_guard<std::mutex>
};

#endif