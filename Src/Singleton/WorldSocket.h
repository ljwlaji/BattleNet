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
	int num;//记录socket的真实数目  
	SOCKET socketArray[256];//存放socket的数组  
	std::mutex ListLock;
	uint8 m_Page;
public:
	bool IsFull();
	SOCKET getSocket(int i) { return socketArray[i]; }
	//构造函数中对两个成员变量进行初始化  
	SocketList(uint8 PageCount);
	//往socketArray中添加一个socket  
	void insertSocket(SOCKET s);
	//从socketArray中删除一个socket  
	void deleteSocket(SOCKET s);
	uint8 GetPage() { return m_Page; }
	//将socketArray中的套接字放入fd_list这个结构体中  
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