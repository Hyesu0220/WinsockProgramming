#pragma comment(lib, "ws2_32")

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <cstdio>
#include <cstdlib>

#define SERVERIP "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE 512

// @see	https://docs.microsoft.com/en-us/windows/win32/api/winsock2/
// @see https://docs.microsoft.com/en-us/windows/win32/winsock/complete-client-code

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
void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)& lpMsgBuf, 0, NULL);
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

int main(int argc, char* argv[])
{
	int iResult = 0;

	// 윈속 초기화
	WSADATA wsaData;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
		err_quit("WSAStartup failed");

	// socket()
	SOCKET connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connectSocket == INVALID_SOCKET)
		err_quit("socket failed");

	// 주소 구조체 준비
	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serverAddr.sin_port = htons(SERVERPORT);
	
	// connect()
	iResult = connect(connectSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (iResult == SOCKET_ERROR)
		err_quit("connect failed");

	// 데이터 통신을 위한 변수 준비
	char buf[BUFSIZE + 1];
	int length = 0;

	printf("INVALID_SOCKET VALUE : %d\n", INVALID_SOCKET);

	while (true)
	{
		// 데이터 입력 받기
		printf("\n[보낼 데이터] ");
		if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
			break;

		// '\n' 제거
		length = strlen(buf);
		if (buf[length - 1] == '\n')
			buf[length - 1] = '\0';
		if (strlen(buf) == 0)
			break;

		// 입력한 데이터 보내기
		iResult = send(connectSocket, buf, strlen(buf), 0);
		if (iResult == SOCKET_ERROR)
		{
			err_display("send failed.");
			closesocket(connectSocket);
			break;
		}

		printf("[TCP 클라이언트] %d 바이트를 보냈습니다.\n", iResult);
		memset(&buf, 0, sizeof(buf));

		iResult = RecvFixedSize(connectSocket, buf, iResult, 0);
		if (iResult == SOCKET_ERROR)
		{
			err_display("An error occurred during a RecvFixedSize() call.");
			break;
		}
		else if (iResult == 0)
		{
			err_display("Connection closed\n");
			closesocket(connectSocket);
			break;
		}

		// 받은 데이터 출력
		buf[iResult] = '\0';
		printf("[TCP 클라이언트] %d 바이트를 받았습니다.\n", iResult);
		printf("[받은데이터] %s\n", buf);
	}

	if (connectSocket != INVALID_SOCKET)
		closesocket(connectSocket);

	WSACleanup();
}