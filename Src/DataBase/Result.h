#ifndef __RESULT_H__
#define __RESULT_H__

#include <string>
#include <map>
#include <vector>
class RowInfo : public std::string
{
public:
	RowInfo(const char* str = nullptr);
	RowInfo(const RowInfo& info);
	RowInfo& operator = (const RowInfo& rowinfo);
	RowInfo operator + (const RowInfo& rowinfo);
	bool operator > (const RowInfo& rowinfo);
	bool operator < (const RowInfo& rowinfo);
	bool operator == (const RowInfo& rowinfo);

	char & operator[](int idx);
	~RowInfo(void);
	std::string GetString() { return _str; }
	int GetInt()			{ return atoi(_str); }
	bool GetBool()			{ bool ret = false; atoi(_str) == 0 ? ret = false : ret = true; return ret; }
	float GetFloat()		{ return (float)atof(_str); }
private:
	char* _str;
};

typedef std::map<int, std::vector<RowInfo>> Result;

#endif