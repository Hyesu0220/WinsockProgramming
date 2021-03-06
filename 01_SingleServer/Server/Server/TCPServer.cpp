#pragma comment(lib, "ws2_32")

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <cstdio>
#include <cstdlib>

#define SERVERPORT 9000
#define BUFSIZE 512

// @see	https://docs.microsoft.com/en-us/windows/win32/api/winsock2/
// @see https://docs.microsoft.com/en-us/windows/win32/winsock/complete-server-code

/**
 * 소켓 함수 오류를 Message Box로 출력 후 프로그램을 종료시키는 함수
 *
 * @param msg	소켓 함수 오류와 함께 출력할 메시지 (Caption)
 */
void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)& lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

/**
 * 소켓 함수 오류를 console에 출력시키는 함수
 *
  * @param msg	소켓 함수 오류와 함께 출력할 메시지 (대활호 안의 내용)
 */
void err_displaty(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)& lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

int main(int argc, char* argv[])
{
	int iResult = 0;
	int iSendResult = 0;

	// 윈속 초기화
	WSADATA wsaData;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	// Listen Socket 생성
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET)
		err_quit("socket()");

	// 주소 구조체와 바인딩
	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(SERVERPORT);
	iResult = bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (iResult == SOCKET_ERROR)
		err_quit("bind()");

	// listen()
	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
		err_quit("listen");

	// 통신에 필요한 변수 준비
	SOCKET clientSocket = INVALID_SOCKET;
	SOCKADDR_IN clientAddr;
	int addrlen = 0;
	char buf[BUFSIZE + 1];

	while (true)
	{
		// accept()
		addrlen = sizeof(clientAddr);
		clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &addrlen);
		if (clientSocket == INVALID_SOCKET)
		{
			err_displaty("accept()");
			continue;
		}

		// 접속 정보 출력
		printf("\n[TCP 서버] 클라이언트 접속 : %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

		while (true)
		{
			// 버프 초기화
			memset(&buf, 0, sizeof(buf));

			// 데이터 받기
			iResult = recv(clientSocket, buf, BUFSIZE, 0);
			if (iResult > 0)
			{
				printf("[TCP %d Bytes Recv / %s:%d] %s\n", iResult, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), buf);
				iSendResult = send(clientSocket, buf, iResult, 0);
				if (iSendResult == SOCKET_ERROR)
				{
					err_displaty("send() failed");
					closesocket(clientSocket);
					break;
				}
				//shutdown(clientSocket, SD_SEND);
				//closesocket(clientSocket);
			}
			else if (iResult == 0)
			{
				printf("\n[TCP 서버] 클라이언트 종료 : %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
				break;
			}
			else
			{
				err_displaty("recv() failed");
				closesocket(clientSocket);
				break;
			}
		}
	}

	closesocket(listenSocket);

	WSACleanup();
	return 0;
}