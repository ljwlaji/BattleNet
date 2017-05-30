#include "WorldSocket.h"
#include "DataBase.h"
#include "WorldUpdater.h"
#include "World.h"
#include "Config.h"
#include "Log.h"
#include <iostream>

bool WorldSocketLoadFinished = false;
void StartWorldSocket()
{
	sWorldSocket->StartUp(WorldSocketLoadFinished);
}

void HandleInPut(std::string msg)
{

}

void main()
{
	sLog->OutLog("正在启动日志服务器....");
	sLog->OnStart();
	sLog->OutLog("正在读取配置文件....");

	while (!sConfig->SetSource("BattleNet.conf"))
	{
		sLog->OutBug("无法获取[BattleNet.conf]配置文件,清检查文件完整性");
		sLog->OutBug("按下任意键重新检测");
		getchar();
	}
	sLog->OutLog("正在读取数据库信息....");
	sDataBase->StartUp();

	sLog->OutLog("正在启动网络服务器....");
	std::thread th(&StartWorldSocket);
	th.detach();
	char msg[123];
	while (1)
	{
		if (WorldSocketLoadFinished)
		{
			sWorld->OnStart();
			std::thread tah(&WorldUpdater::StartUpdate);
			tah.detach();
			break;
		}
	}

	while (1)
	{
		if (WorldSocketLoadFinished)
		{
			std::cin >> msg;
			HandleInPut(msg);
		}
		else Sleep(1);
	}
}
