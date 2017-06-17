#include "WorldSocket.h"
#include "Opcodes.h"
#include "World.h"
#include "WorldPacket.h"
#include "Config.h"
#include "Log.h"

WorldSocket* _WorldSocket = nullptr;

WorldSocket::WorldSocket() : ServerIp(""),ServerPort(0)
{
	SocketNumber = sConfig->GetIntDefault("BattleNetWorkThread", 1);
	ServerIp = sConfig->GetStringDefault("BattleNetServerIP", "127.0.0.1").c_str();
	ServerPort = sConfig->GetIntDefault("BattleNetServerPort", 6000);
	sLog->OutLog("服务器IP设置为%s", ServerIp.c_str());
	sLog->OutLog("服务器端口设置为%d", ServerPort);
}

WorldSocket::~WorldSocket()
{
	_WorldSocket = nullptr;
}

WorldSocket * WorldSocket::GetInstance()
{
	if (!_WorldSocket)
		_WorldSocket = new WorldSocket();
	return _WorldSocket;
}

int WorldSocket::SendPacket(SOCKET socket, const WorldPacket & pack)
{
	char send_buffer[1024];
	int send_len = 8 + pack.size();
	*((int*)send_buffer) = send_len;
	*((int*)(send_buffer + 4)) = pack.GetOpcode();
	memcpy(send_buffer + 8, pack.contents(), pack.size());

	return send(socket, send_buffer, send_len, 0);
}

void WorldSocket::StartUp(bool& Finished)
{
	WSADATA wsaData;
	int err;

	//1.加载套接字库  
	err = WSAStartup(MAKEWORD(1, 1), &wsaData);
	if (err != 0)
	{
		sLog->OutBug("套接字服务端启动失败,错误代码 :%d", GetLastError());
		return;
	}

	//2.创建socket  
	//套接字描述符,SOCKET实际上是unsigned int  
	SOCKET serverSocket;
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET)
	{
		sLog->OutBug("套接字服务端启动失败,错误代码 :%d", GetLastError());
		return;
	}


	//服务器端的地址和端口号  
	struct sockaddr_in serverAddr, clientAdd;
	serverAddr.sin_addr.s_addr = inet_addr(ServerIp.c_str());
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(ServerPort);

	//3.绑定Socket，将Socket与某个协议的某个地址绑定  
	err = ::bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if (err != 0)
	{
		sLog->OutBug("套接字服务端绑定失败,错误代码 :%d", GetLastError());
		return;
	}


	//4.监听,将套接字由默认的主动套接字转换成被动套接字  
	err = listen(serverSocket, 10);
	if (err != 0)
	{
		sLog->OutBug("套接字服务端监听失败,错误代码 :%d", GetLastError());
		return;
	}

	sLog->OutLog("套接字服务端启动成功...正在创建线程...");
	int addrLen = sizeof(clientAdd);
	SOCKET sockConn;

	SocketListVector.clear();
	sLog->OutWarning("网络线程数量被设置为：%d 个", SocketNumber);
	for (int i = 0; i != SocketNumber; i++)
	{
		SocketList* socketList = new SocketList(i);
		PushList(socketList);
		std::thread th(&WorldSocket::StartNewWorkThread, this, socketList);
		th.detach();
	}
	sLog->OutSuccess("网络线程创建完毕", SocketNumber);
	Finished = true;
	while (true)
	{
		//5.接收请求，当收到请求后，会将客户端的信息存入clientAdd这个结构体中，并返回描述这个TCP连接的Socket  
		sockConn = accept(serverSocket, (struct sockaddr*)&clientAdd, &addrLen);
		if (sockConn == INVALID_SOCKET)
		{
			sLog->OutBug("套接字服务端监听失败,错误代码 :%d", GetLastError());
			return;
		}
		bool AllFull = true;
		for (int i = 0; i < SocketNumber; i++)
		{

			if (!SocketListVector.at(i)->IsFull())
			{
				SocketList* TempList = SocketListVector.at(i);
				SocketListVector.at(i)->insertSocket(sockConn);
				//struct sockaddr_in sa;
				//int len = sizeof(sa);
				//if (!getpeername(sockConn, (sockaddr*)&sa, &len))
				sLog->OutLog("新连接接入 %d", sockConn);
				NewSocketComming(sockConn, i);
				AllFull = false;
				break;
			}
		}

		if (AllFull)
			sLog->OutLog("连接数达到上限");
		//将之前的第6步替换成了上面启动workThread这个线程函数和下面这一行代码  
		//将socket放入socketList中  
	}

	closesocket(serverSocket);
	//清理Windows Socket库  
	WSACleanup();
}

void WorldSocket::StartNewWorkThread(SocketList* pList)
{
	//传递进来的socketList指针  
	SocketList* socketList = pList;
	int ErrorCode = 0;
	fd_set fdread;//存在读文件的set，select会检测这个set中是否可以从某些socket中读入信息  

	struct timeval timeout;//设置select超时的时间  
	timeout.tv_sec = 6;
	timeout.tv_usec = 0;

	SOCKET socket;

	while (true)
	{
		socketList->makefd(&fdread);
		ErrorCode = select(0, &fdread, NULL, NULL, &timeout);
		if (ErrorCode == 0)//select返回0表示超时
		{
			sLog->OutLog("线程 %d 暂无可用数据包接收", pList->GetPage());
			continue;
		}
		else if (ErrorCode == -1)
		{
			Sleep(1);
		}
		else
		{
			//遍历socketList中的每一个socket，查看那些socket是可读的，处理可读的socket  
			//从中读取数据到缓冲区，并发送数据给客户端  
			for (int i = 0; i < FD_SETSIZE; i++)
			{
				//读取有效的socket
				socket = socketList->getSocket(i);
				char receBuff[4096];
				if (!socket)
					continue;
				//判断哪些socket是可读的，如果这个socket是可读的，从它里面读取数据  
				if (FD_ISSET(socket, &fdread))
				{
					ErrorCode = recv(socket, receBuff, sizeof(receBuff), 0);
					//如果返回值表示要关闭这个连接，那么关闭它，并将它从sockeList中去掉  
					if (ErrorCode == 0 || ErrorCode == SOCKET_ERROR)
					{
						if (WorldSession * session = sWorld->GetSessionBySocket(socket))
						{
							session->Close();
							continue;
						}
						closesocket(socket);
						sLog->OutLog("客户端取消连接,%u", socket);
						socketList->deleteSocket(socket);
					}
					else
					{
						RecvPacket(socket, receBuff);
					}
				}
			}
		}
	}
	return;
}

void WorldSocket::PushList(SocketList * pList)
{
	SocketListVector.push_back(pList);
}

void WorldSocket::NewSocketComming(const SOCKET & pSocket,const uint8& SocketPage)
{
	struct sockaddr_in sa;
	int len = sizeof(sa);
	getpeername(pSocket, (sockaddr*)&sa, &len);
	std::string address = inet_ntoa(sa.sin_addr);
	if (WorldSession* session = sWorld->CreateSessionForNewSocket(pSocket, address,SocketPage))
		SocketMap[pSocket] = session;
}

void WorldSocket::CloseSocket(const SOCKET & pSocket, const uint8& socketpage)
{
	closesocket(pSocket);
	SocketListVector.at(socketpage)->deleteSocket(pSocket);
}


//void WorldSocket::RemoveSocket(const SOCKET & socket)
//{
//}

void WorldSocket::RecvPacket(const SOCKET& pSocket, const char * buff)
{
	uint16 Opcode = *((uint16*)(buff + 4));
	if (!Opcode || Opcode > 0x424)
		return;
	ThreadLocker loc(SocketMapLock);
	if (SocketMap.find(pSocket) == SocketMap.end())
		return;
	if (WorldSession* TempSession = sWorld->GetSessionBySocket(pSocket))
		TempSession->PushPacket(buff);
}

SocketList::SocketList(uint8 PageCount) : m_Page(PageCount)
{
	ThreadLocker loc(ListLock);
	num = 0;
	for (int i = 0; i < FD_SETSIZE; i++)
		//因为socket的值是一个非负整数值，所以可以将socketArray初始化为0，让它来表示数组中的这一个元素有没有被使用  
		socketArray[i] = 0;
}

bool SocketList::IsFull()
{
	bool IsFull = true;
	ThreadLocker loc(ListLock);
	socketArray[FD_SETSIZE - 1] == 0 ? IsFull = false : IsFull = true;
	return IsFull;
}

void SocketList::insertSocket(SOCKET s)
{
	ThreadLocker loc(ListLock);
	for (int i = 0; i < FD_SETSIZE; i++)
	{
		//如果某一个socketArray[i]为0，表示哪一个位可以放入socket
		if (socketArray[i] == 0)
		{
			socketArray[i] = s;
			num++;
			break;//这里一定要加上break，不然一个socket会放在socketArray的多个位置上  
		}
	}
}

void SocketList::deleteSocket(SOCKET s)
{
	ThreadLocker loc(ListLock);
	for (int i = 0; i < FD_SETSIZE; i++)
	{
		if (socketArray[i] == s)
		{
			socketArray[i] = 0;
			num--;
			sLog->OutLog("移除客户端 %d, 目前容量 %d", s, FD_SETSIZE - num);
			return;
		}
	}
}

void SocketList::makefd(fd_set* fd_list)
{
	ThreadLocker loc(ListLock);
	FD_ZERO(fd_list);//首先将fd_list清0  
					 //将每一个socket加入fd_list中  
	for (int i = 0; i < FD_SETSIZE; i++)
		if (socketArray[i]>0)
			FD_SET(socketArray[i], fd_list);
}