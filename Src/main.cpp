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
	sLog->OutLog("����������־������....");
	sLog->OnStart();
	sLog->OutLog("���ڶ�ȡ�����ļ�....");

	while (!sConfig->SetSource("BattleNet.conf"))
	{
		sLog->OutBug("�޷���ȡ[BattleNet.conf]�����ļ�,�����ļ�������");
		sLog->OutBug("������������¼��");
		getchar();
	}
	sLog->OutLog("���ڶ�ȡ���ݿ���Ϣ....");
	sDataBase->StartUp();

	sLog->OutLog("�����������������....");
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
