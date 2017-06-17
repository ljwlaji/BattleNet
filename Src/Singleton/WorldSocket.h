#ifndef __WORLD_SOCKET_H__
#define __WORLD_SOCKET_H__

#ifdef _MSC_VER
#ifdef _WIN64
#include <WinSock2.h>
#elif _WIN32
#include <winsock.h>
#endif
#endif
#pragma comment(lib, "ws2_32.lib")
#include "Common.h"

#define sWorldSocket WorldSocket::GetInstance()

class SocketList
{
private:
	int num;//��¼socket����ʵ��Ŀ  
	SOCKET socketArray[256];//���socket������  
	std::mutex ListLock;
	uint8 m_Page;
public:
	bool IsFull();
	SOCKET getSocket(int i) { return socketArray[i]; }
	//���캯���ж�������Ա�������г�ʼ��  
	SocketList(uint8 PageCount);
	//��socketArray�����һ��socket  
	void insertSocket(SOCKET s);
	//��socketArray��ɾ��һ��socket  
	void deleteSocket(SOCKET s);
	uint8 GetPage() { return m_Page; }
	//��socketArray�е��׽��ַ���fd_list����ṹ����  
	void makefd(fd_set * fd_list);
};

class WorldPacket;
class WorldSession;
class WorldSocket
{
public:
	static WorldSocket* GetInstance();
public:
	int SendPacket(SOCKET socket,const WorldPacket& pack);
	void StartUp(bool& Finished);
	void StartNewWorkThread(SocketList* pList);
	void PushList(SocketList* pList);
	void NewSocketComming(const SOCKET& pSocket,const uint8& SOcketPage);
	void CloseSocket(const SOCKET& pSocket,const uint8& socketpage);
	//void RemoveSocket(const SOCKET& socket);
private:
	WorldSocket();
	~WorldSocket();
	void RecvPacket(const SOCKET& socket, const char *buff);
	std::vector<SocketList*> SocketListVector;
	std::map<SOCKET, WorldSession*> SocketMap;
	mutex SocketMapLock;
	uint8 SocketNumber;
	std::string ServerIp;
	uint16 ServerPort;
};

#endif