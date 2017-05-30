#include "DataBase.h"
#include "Config.h"
#include "Log.h"
static DataBase* _DataBase = nullptr;

DataBase * DataBase::GetInstance()
{
	if (!_DataBase)
		_DataBase = new DataBase();
	return _DataBase;
}

DataBase::DataBase()
{
	
}

DataBase::~DataBase()
{

}

void DataBase::StartUp()
{
	server		= sConfig->GetStringDefault("DataBaseAddress", "");
	user		= sConfig->GetStringDefault("DataBaseUserName", "");
	password	= sConfig->GetStringDefault("DataBasePassWord", "");
	port		= sConfig->GetIntDefault("DataBaseProt", 3306);

	sLog->OutWarning("***************************************************************************");
	sLog->OutWarning("**数据库设置信息:");
	sLog->OutWarning("**IP地址:%s", server.c_str());
	sLog->OutWarning("**端口号:%d", port);
	sLog->OutWarning("**用户名:%s", user.c_str());
	sLog->OutWarning("**密码  :%s", password.c_str());
	sLog->OutWarning("***************************************************************************");
}

std::string DataBase::GetDataBaseName(DataBaseType _type)
{
	std::string res = "";
	switch (_type)
	{
	case CharacterDataBase: res = sConfig->GetStringDefault("CharacterDatabase", "");
		break;
	case LoginDataBase:
		res = sConfig->GetStringDefault("LoginDataBase", "");
		break;
	case WorldDataBase:
		res = sConfig->GetStringDefault("WorldDataBase", "");
		break;
	}
	return res.c_str();
}

void DataBase::PExcude(DataBaseType _type, const char* args, ...)
{
	_database = GetDataBaseName(_type).c_str();
	if (_database.empty())
		return;
	va_list ap;
	char szQuery[MAX_QUERY_LEN];
	va_start(ap, args);
	int res = vsnprintf(szQuery, MAX_QUERY_LEN, args, ap);
	va_end(ap);

	if (res == -1)
		return;

	MYSQL mysql;//连接数据库的变量
	int t;
	mysql_init(&mysql);

	if (!mysql_real_connect(&mysql, server.c_str(), user.c_str(), password.c_str(), _database.c_str(), port, NULL, 0))
	{
		mysql_close(&mysql);
		return;
	}
	mysql_query(&mysql, "SET NAMES UTF8");
	t = mysql_real_query(&mysql, args, (unsigned int)strlen(args));
	mysql_close(&mysql);
}

bool DataBase::GetResult(DataBaseType _type, Result& sqlresult, const char* args, ...)
{
	_database = GetDataBaseName(_type).c_str();
	if (_database.empty())
		return false;
	va_list ap;
	char szQuery[MAX_QUERY_LEN];
	va_start(ap, args);
	int ares = vsnprintf(szQuery, MAX_QUERY_LEN, args, ap);
	va_end(ap);

	if (ares == -1)
		return false;

	sqlresult.clear();
	MYSQL mysql;//连接数据库的变量
	MYSQL_RES *res;//存放查询结果的变量
	MYSQL_ROW row;
	unsigned int t;
	mysql_init(&mysql);
	if (!mysql_real_connect(&mysql, server.c_str(), user.c_str(), password.c_str(), _database.c_str(), port, NULL, 0))
	{
		mysql_close(&mysql);
		return nullptr;
	}
	mysql_query(&mysql, "SET NAMES UTF8");
	t = mysql_real_query(&mysql, args, (unsigned int)strlen(args));
	if (t)
	{
		return nullptr;
	}
	else
		res = mysql_store_result(&mysql);
	unsigned int i = 0;
	while (row = mysql_fetch_row(res))
	{
		std::vector<RowInfo> _info;
		for (t = 0; t <= mysql_num_fields(res); t++)
		{
			RowInfo rowinfo = row[t];
			_info.push_back(rowinfo);
		}
		sqlresult[i] = _info;
		i++;
	}
	mysql_free_result(res);
	mysql_close(&mysql);
	return true;
}