#pragma once
#pragma comment(lib, "ws2_32")

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <WinSock2.h>
#include <Windows.h>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>
#include <mutex>

#define SERVERPORT		9000
#define BUFSIZE			512
#define MAXUSER			10

using std::shared_ptr; 
using std::thread;


void error_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void error_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

int RecvFixedSize(SOCKET in_socket, char* inout_buffer, int in_fixedsize, int flags)
{
	int recievedSize = 0;
	char* currentBufPosition = inout_buffer;
	int leftSize = in_fixedsize;

	while (leftSize > 0)
	{
		recievedSize = recv(in_socket, currentBufPosition, leftSize, flags);
		if (recievedSize == SOCKET_ERROR)
		{
			return SOCKET_ERROR;
		}
		else if (recievedSize == 0)
		{
			// 접속 종료됨을 의미
			break;
		}

		leftSize -= recievedSize;
		currentBufPosition += recievedSize;
	}
	return (in_fixedsize - leftSize);
}

struct User
{
	SOCKET mSocket;
	SOCKADDR_IN mAddr;
	HANDLE mRecvEvents[2];
	HANDLE mSendEvents[2];
	char mBuf[BUFSIZE + 1];
	int mRecvSize;

	HANDLE mRecvThreadKilled;
	HANDLE mSendThreadKilled;
	shared_ptr<thread> RecvThreadPtr;
	shared_ptr<thread> SendThreadPtr;

	User()
	{
		mSocket = INVALID_SOCKET;
		ZeroMemory(&mAddr, sizeof(mAddr));
		for (int i = 0; i < 2; i++)
		{
			mRecvEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
		}

		for (int i = 0; i < 2; i++)
		{
			mSendEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
		}

		memset(&mBuf, 0, sizeof(mBuf));
		mRecvSize = 0;
		RecvThreadPtr.reset();
		SendThreadPtr.reset();
	}
	
	~User()
	{
		SendThreadPtr->detach();
		RecvThreadPtr->detach();
		for (int i = 0; i < 2; i++)
		{
			CloseHandle(mSendEvents[i]);
		}

		for (int i = 0; i < 2; i++)
		{
			CloseHandle(mRecvEvents[i]);
		}
	}

	void ResetBuf()
	{
		memset(&mBuf, 0, sizeof(mBuf));
		mRecvSize = 0;
	}
};

std::vector<shared_ptr<User>> Users;

shared_ptr<User> CreateUser(SOCKET InSocket, SOCKADDR_IN InAddr);
void RemoveUser(shared_ptr<User> InUser);

void RecvThread(shared_ptr<User> InUser)
{
	int iResult = 0;
	int iEventIndex = 0;
	shared_ptr<User> user = InUser;

	while (true)
	{
		iEventIndex = WaitForMultipleObjects(2, user->mRecvEvents, FALSE, INFINITE);
		if (iEventIndex == WAIT_FAILED)
		{
			error_display("Failed to WaitForSingleObject()");
			break;
		}

		ResetEvent(user->mRecvEvents[iEventIndex -= WAIT_OBJECT_0]);
		if (iEventIndex == 1)
			break;
		
		iResult = RecvFixedSize(user->mSocket, (char*)&user->mRecvSize, sizeof(int), 0);
		if (iResult == SOCKET_ERROR)
		{
			error_display("Failed to receive data size.");
			break;
		}
		else if (iResult == 0)
		{
			break;
		}

		iResult = RecvFixedSize(user->mSocket, user->mBuf, user->mRecvSize, 0);
		if (iResult == SOCKET_ERROR)
		{
			error_display("Failed to receive data.");
			break;
		}
		else if (iResult == 0)
		{
			break;
		}

		user->mBuf[user->mRecvSize] = '\0';
		printf("[TCP/%s:%d] %s\n", inet_ntoa(user->mAddr.sin_addr), ntohs(user->mAddr.sin_port), user->mBuf);
		SetEvent(user->mSendEvents[0]);
	}

	// Remove in Users
	if (Users.size() > 0)
	{
		for (auto iter = Users.begin(); iter != Users.end(); ++iter)
		{
			if (*iter == user)
			{
				RemoveUser(user);
				break;
			}
		}
	}		
}

void SendThread(shared_ptr<User> InUser)
{
	int iResult = 0;
	int iEventIndex = 0;
	shared_ptr<User> user = InUser;

	while (true)
	{
		iEventIndex = WaitForMultipleObjects(2, user->mSendEvents, FALSE, INFINITE);
		if (iEventIndex == WAIT_FAILED)
		{
			error_display("Failed to WaitForSingleObject()");
			break;
		}

		ResetEvent(user->mSendEvents[iEventIndex -= WAIT_OBJECT_0]);		
		if (iEventIndex == 1)
			break;
		
		int iResut = send(user->mSocket, user->mBuf, user->mRecvSize, 0);
		if (iResut == SOCKET_ERROR)
		{
			char* msg = new char();
			sprintf(msg, "%s:%d] Failed send()", inet_ntoa(user->mAddr.sin_addr), ntohs(user->mAddr.sin_port));
			error_display(msg);
			delete msg;
			break;
		}

		user->ResetBuf();
		SetEvent(user->mRecvEvents[0]);
	}
	
	// Remove in Users
	if (Users.size() > 0)
	{
		for (auto iter = Users.begin(); iter != Users.end(); ++iter)
		{
			if (*iter == user)
			{
				RemoveUser(user);
				break;
			}
		}
	}
}

shared_ptr<User> CreateUser(SOCKET InSocket, SOCKADDR_IN InAddr)
{
	printf("\n[TCP 서버] 클라이언트 접속 : %s:%d\n", inet_ntoa(InAddr.sin_addr), ntohs(InAddr.sin_port));
	shared_ptr<User> user = std::make_shared<User>();
	user->mSocket = InSocket;
	user->mAddr = InAddr;
	Users.push_back(user);
	printf("\n[현재 총 인원 %d]\n", Users.size());
	return user;
}

void RemoveUser(shared_ptr<User> InUser)
{
	printf("\n[TCP 서버] 클라이언트 종료 : %s:%d\n", inet_ntoa(InUser->mAddr.sin_addr), ntohs(InUser->mAddr.sin_port));
	for(auto iter = Users.begin() ; iter != Users.end(); ++iter)
	{
		if (*iter == InUser)
		{
			Users.erase(iter);
			break;
		}
	}
	// Thread 종료
	SetEvent(InUser->mRecvEvents[1]);
	SetEvent(InUser->mSendEvents[1]);

	closesocket(InUser->mSocket);
	InUser.reset();
}

int main()
{
	int iResult = 0;
	WSADATA wsaData;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		error_quit("WSAStartup failed with error");
	}

	SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET)
	{
		error_quit("Failed to create 'listen socket'.");
	}

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(ServerAddr));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(SERVERPORT);
	ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	iResult = bind(ListenSocket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr));
	if (iResult == SOCKET_ERROR)
		error_quit("Failed bind()");

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
		error_quit("Failed listen()");

	while (true)
	{
		SOCKET ClientSocket = INVALID_SOCKET;
		SOCKADDR_IN ClientAddr;
		ZeroMemory(&ClientAddr, sizeof(ClientAddr));
		int Addrlen = sizeof(ClientAddr);

		ClientSocket = accept(ListenSocket, (SOCKADDR*)& ClientAddr, &Addrlen);
		if (ClientSocket == INVALID_SOCKET)
		{
			error_display("Failed to accept()");
			continue;
		}
		shared_ptr<User> user = CreateUser(ClientSocket, ClientAddr);
		if (user != nullptr)
		{
			user->RecvThreadPtr = std::make_shared<thread>(RecvThread, user);
			user->SendThreadPtr = std::make_shared<thread>(SendThread, user);
			SetEvent(user->mRecvEvents[0]);
		}
	}	

	return 0;
}