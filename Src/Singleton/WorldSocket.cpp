#include "WorldSocket.h"
#include "Opcodes.h"
#include "World.h"
#include "WorldPacket.h"
#include "Config.h"
#include "Log.h"

WorldSocket* _WorldSocket = nullptr;

WorldSocket::WorldSocket()
{
	SocketNumber = sConfig->GetIntDefault("BattleNetWorkThread", 1);
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

void WorldSocket::StartUp(bool& Finished)
{
	WSADATA wsaData;
	int err;

	//1.�����׽��ֿ�  
	err = WSAStartup(MAKEWORD(1, 1), &wsaData);
	if (err != 0)
	{
		sLog->OutBug("�׽��ַ��������ʧ��,������� :%d", GetLastError());
		return;
	}

	//2.����socket  
	//�׽���������,SOCKETʵ������unsigned int  
	SOCKET serverSocket;
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET)
	{
		sLog->OutBug("�׽��ַ��������ʧ��,������� :%d", GetLastError());
		return;
	}


	//�������˵ĵ�ַ�Ͷ˿ں�  
	struct sockaddr_in serverAddr, clientAdd;
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(6000);

	//3.��Socket����Socket��ĳ��Э���ĳ����ַ��  
	err = ::bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if (err != 0)
	{
		sLog->OutBug("�׽��ַ���˰�ʧ��,������� :%d", GetLastError());
		return;
	}


	//4.����,���׽�����Ĭ�ϵ������׽���ת���ɱ����׽���  
	err = listen(serverSocket, 10);
	if (err != 0)
	{
		sLog->OutBug("�׽��ַ���˼���ʧ��,������� :%d", GetLastError());
		return;
	}

	sLog->OutLog("�׽��ַ���������ɹ�...���ڴ����߳�...");
	int addrLen = sizeof(clientAdd);
	SOCKET sockConn;

	SocketListVector.clear();
	sLog->OutWarning("�����߳�����������Ϊ��%d ��", SocketNumber);
	for (int i = 0; i != SocketNumber; i++)
	{
		SocketList* socketList = new SocketList();
		PushList(socketList);
		std::thread th(&WorldSocket::StartNewWorkThread, this, socketList);
		th.detach();
	}
	sLog->OutSuccess("�����̴߳������", SocketNumber);
	Finished = true;
	while (true)
	{
		//5.�������󣬵��յ�����󣬻Ὣ�ͻ��˵���Ϣ����clientAdd����ṹ���У��������������TCP���ӵ�Socket  
		sockConn = accept(serverSocket, (struct sockaddr*)&clientAdd, &addrLen);
		if (sockConn == INVALID_SOCKET)
		{
			sLog->OutBug("�׽��ַ���˼���ʧ��,������� :%d", GetLastError());
			return;
		}
		bool AllFull = true;
		for (int i = 0; i < SocketNumber; i++)
		{

			if (!SocketListVector.at(i)->IsFull())
			{
				SocketList* TempList = SocketListVector.at(i);
				SocketListVector.at(i)->insertSocket(sockConn);
				struct sockaddr_in sa;
				int len = sizeof(sa);
				if (!getpeername(sockConn, (sockaddr*)&sa, &len))
					sLog->OutLog("�����ӽ���: %s", inet_ntoa(sa.sin_addr));
				AllFull = false;
				break;
			}
		}

		if (AllFull)
			sLog->OutLog("�������ﵽ����");
		//��֮ǰ�ĵ�6���滻������������workThread����̺߳�����������һ�д���  
		//��socket����socketList��  
	}

	closesocket(serverSocket);
	//����Windows Socket��  
	WSACleanup();
}

void WorldSocket::StartNewWorkThread(SocketList* pList)
{
	sLog->OutSuccess("�����߳̿�ʼ����...");
	//���ݽ�����socketListָ��  
	SocketList* socketList = pList;
	int ErrorCode = 0;
	fd_set fdread;//���ڶ��ļ���set��select�������set���Ƿ���Դ�ĳЩsocket�ж�����Ϣ  

	struct timeval timeout;//����select��ʱ��ʱ��  
	timeout.tv_sec = 6;
	timeout.tv_usec = 0;

	SOCKET socket;

	while (true)
	{
		socketList->makefd(&fdread);
		ErrorCode = select(0, &fdread, NULL, NULL, &timeout);
		if (ErrorCode == 0)//select����0��ʾ��ʱ
		{
			sLog->OutLog("���޿������ݰ�����");
			continue;
		}
		else if (ErrorCode == -1)
		{
			Sleep(1);
		}
		else
		{
			//����socketList�е�ÿһ��socket���鿴��Щsocket�ǿɶ��ģ�����ɶ���socket  
			//���ж�ȡ���ݵ������������������ݸ��ͻ���  
			for (int i = 0; i < FD_SETSIZE; i++)
			{
				//��ȡ��Ч��socket
				socket = socketList->getSocket(i);
				char receBuff[4096];
				if (!socket)
					continue;
				//�ж���Щsocket�ǿɶ��ģ�������socket�ǿɶ��ģ����������ȡ����  
				if (FD_ISSET(socket, &fdread))
				{
					ErrorCode = recv(socket, receBuff, sizeof(receBuff), 0);
					//�������ֵ��ʾҪ�ر�������ӣ���ô�ر�������������sockeList��ȥ��  
					if (ErrorCode == 0 || ErrorCode == SOCKET_ERROR)
					{
						closesocket(socket);
						sLog->OutLog("�ͻ���ȡ������,%u", socket);
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

void WorldSocket::NewSocketComming(const SOCKET & pSocket, const char * buff)
{
	if (WorldSession* session = sWorld->CreateSessionForNewSocket(pSocket, buff))
		SocketMap[pSocket] = session;
}


//void WorldSocket::RemoveSocket(const SOCKET & socket)
//{
//}

void WorldSocket::RecvPacket(const SOCKET& pSocket, const char * buff)
{
	uint16 Opcode = *((uint8*)(buff + 4));
	if (Opcode > 0x424)
		return;
	ThreadLocker loc(SocketMapLock);
	if (SocketMap.find(pSocket) == SocketMap.end())
	{//Create A TempThread For New Socket
		std::thread NesSocketThread(&WorldSocket::NewSocketComming, this, pSocket, buff);
		NesSocketThread.detach();
		return;
	}
	if (WorldSession* TempSession = sWorld->GetSessionBySocket(pSocket))
		TempSession->PushPacket(buff);
}

SocketList::SocketList()
{
	ThreadLocker loc(ListLock);
	num = 0;
	for (int i = 0; i < FD_SETSIZE; i++)
		//��Ϊsocket��ֵ��һ���Ǹ�����ֵ�����Կ��Խ�socketArray��ʼ��Ϊ0����������ʾ�����е���һ��Ԫ����û�б�ʹ��  
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
		//���ĳһ��socketArray[i]Ϊ0����ʾ��һ��λ���Է���socket
		if (socketArray[i] == 0)
		{
			socketArray[i] = s;
			num++;
			break;//����һ��Ҫ����break����Ȼһ��socket�����socketArray�Ķ��λ����  
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
			break;
		}
	}
}

void SocketList::makefd(fd_set* fd_list)
{
	ThreadLocker loc(ListLock);
	FD_ZERO(fd_list);//���Ƚ�fd_list��0  
					 //��ÿһ��socket����fd_list��  
	for (int i = 0; i < FD_SETSIZE; i++)
		if (socketArray[i]>0)
			FD_SET(socketArray[i], fd_list);
}