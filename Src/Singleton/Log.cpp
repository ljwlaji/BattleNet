#include "Log.h"
#include <Windows.h>
#include <iostream>

static Log* _log = nullptr;
static mutex LocInsLoc;

Log * Log::GetInstance()
{
	if (!_log)
	{
		ThreadLocker loc(LocInsLoc);
		if (!_log)
			_log = new Log();
	}
	return _log;
}

void Log::OutBug(const char * args,...)
{
	va_list ap;
	char szQuery[1024];
	va_start(ap, args);
	int ares = vsnprintf(szQuery, 1024, args, ap);
	va_end(ap);

	if (ares == -1)
		return;

	SetColor(Cmd_Red);
	cout << szQuery << endl;
	ReSetColor();
}

void Log::OutLog(const char * args, ...)
{
	va_list ap;
	char szQuery[1024];
	va_start(ap, args);
	int ares = vsnprintf(szQuery, 1024, args, ap);
	va_end(ap);

	if (ares == -1)
		return;

	SetColor(Cmd_White);
	cout << szQuery << endl;
	ReSetColor();
}

void Log::OutSuccess(const char * args, ...)
{
	va_list ap;
	char szQuery[1024];
	va_start(ap, args);
	int ares = vsnprintf(szQuery, 1024, args, ap);
	va_end(ap);

	if (ares == -1)
		return;

	SetColor(Cmd_Green);
	cout << szQuery << endl;
	ReSetColor();
}

void Log::OutWarning(const char * args,...)
{
	va_list ap;
	char szQuery[1024];
	va_start(ap, args);
	int ares = vsnprintf(szQuery, 1024, args, ap);
	va_end(ap);

	if (ares == -1)
		return;

	SetColor(Cmd_Yellow);
	cout << szQuery << endl;
	ReSetColor();
}

void Log::OnStart()
{
	SetColor(Cmd_Pink);
	OutLog("日志服务启动");
	ReSetColor();
}

void Log::ReSetColor()
{
	SetColor(Cmd_Normal);
}

void Log::SetColor(LogColor _color)
{
	switch (_color)
	{
	case Cmd_Normal:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY);
		break;
	case Cmd_White:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		break;
	case Cmd_Red:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
		break;
	case Cmd_Green:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
		break;
	case Cmd_Blue:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE);
		break;
	case Cmd_Yellow:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
		break;
	case Cmd_Pink:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE);
		break;
	}
}

Log::Log()
{

}

Log::~Log()
{
}