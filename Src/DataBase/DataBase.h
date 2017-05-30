#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <Windows.h>
#include <mysql.h>
#include "Result.h"
#define MAX_QUERY_LEN 1024
#define sDataBase DataBase::GetInstance()

enum DataBaseType
{
	CharacterDataBase,
	LoginDataBase,
	WorldDataBase,
};

class DataBase
{
public:
	static DataBase* GetInstance();
	bool GetResult(DataBaseType _type, Result& _res, const char* format, ...);
	void PExcude(DataBaseType _type, const char* args, ...);
	void StartUp();
private:
	DataBase();
	~DataBase();
	std::string GetDataBaseName(DataBaseType _type);
	std::string _database;
	std::string server;
	std::string user;
	std::string password;
	int port;
};


#endif