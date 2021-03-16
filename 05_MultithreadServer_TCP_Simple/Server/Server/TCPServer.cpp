#pragma once
#pragma comment(lib, "ws2_32")

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>
#include <mutex>

//using namespace std;

#define SERVERPORT	9000
#define BUFSIZE		512


void error_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)& lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void error_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)& lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
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

void WorkerThread(LPVOID arg)
{
	SOCKET ClientSocket = (SOCKET)arg;
	int iResult = 0;
	SOCKADDR_IN ClientAddr;
	int AddrLen = 0;
	char Buf[BUFSIZE + 1];
	memset(&Buf, 0, sizeof(Buf));

	AddrLen = sizeof(ClientAddr);
	getpeername(ClientSocket, (SOCKADDR*)& ClientAddr, &AddrLen);

	while (true)
	{
		int RecvSize = 0;
		// 데이터 크기 받기
		iResult = RecvFixedSize(ClientSocket, (char*)&RecvSize, sizeof(int), 0);
		if (iResult == SOCKET_ERROR)
		{
			error_display("Failed to receive data size.");
			break;
		}
		else if (iResult == 0)
		{
			printf("\n[TCP 서버] 클라이언트 종료 : %s:%d\n", inet_ntoa(ClientAddr.sin_addr), ntohs(ClientAddr.sin_port));
			break;
		}

		// 데이터 받기
		iResult = RecvFixedSize(ClientSocket, Buf, RecvSize, 0);
		if (iResult == SOCKET_ERROR)
		{
			error_display("Failed to receive  data.");
			break;
		}
		else if (iResult == 0)
		{
			printf("\n[TCP 서버] 클라이언트 종료 : %s:%d\n", inet_ntoa(ClientAddr.sin_addr), ntohs(ClientAddr.sin_port));
			break;
		}

		Buf[iResult] = '\0';
		printf("[TCP/%s:%d] %s\n", inet_ntoa(ClientAddr.sin_addr), ntohs(ClientAddr.sin_port), Buf);
	}

	closesocket(ClientSocket);
	return;
}

int main()
{
	int iResult = 0;
	std::vector<std::shared_ptr<std::thread>> Threads;

	WSADATA wsaData;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		error_quit("WSAStartup failed with error");
	}

	SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET)
		error_quit("Failed to create 'listen socket'.");

	SOCKADDR_IN ServerSocketAddr;
	ZeroMemory(&ServerSocketAddr, sizeof(ServerSocketAddr));
	ServerSocketAddr.sin_family = AF_INET;
	ServerSocketAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	ServerSocketAddr.sin_port = htons(SERVERPORT);

	iResult = bind(ListenSocket, (SOCKADDR*)&ServerSocketAddr, sizeof(ServerSocketAddr));
	if (iResult == SOCKET_ERROR)
		error_quit("Failed bind()");

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
		error_quit("Failed to listen()");

	while (true)
	{
		SOCKET ClientSocket = INVALID_SOCKET;
		SOCKADDR_IN ClientAddr;
		ZeroMemory(&ClientAddr, sizeof(ClientAddr));
		int AddrLen = sizeof(ClientAddr);

		ClientSocket = accept(ListenSocket, (SOCKADDR*)& ClientAddr, &AddrLen);
		if (ClientSocket == INVALID_SOCKET)
		{
			error_display("Failed to accept()");
			continue;
		}

		// 접속 정보 출력
		printf("\n[TCP 서버] 클라이언트 접속 : %s:%d\n", inet_ntoa(ClientAddr.sin_addr), ntohs(ClientAddr.sin_port));

		std::shared_ptr<std::thread> Worker = std::make_shared<std::thread>(WorkerThread, (LPVOID)ClientSocket);
		Threads.push_back(std::move(Worker));
		continue;
	}

	closesocket(ListenSocket);

	WSACleanup();
	return 0;
}