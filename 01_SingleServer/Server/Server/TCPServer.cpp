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
 * ���� �Լ� ������ Message Box�� ��� �� ���α׷��� �����Ű�� �Լ�
 *
 * @param msg	���� �Լ� ������ �Բ� ����� �޽��� (Caption)
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
 * ���� �Լ� ������ console�� ��½�Ű�� �Լ�
 *
  * @param msg	���� �Լ� ������ �Բ� ����� �޽��� (��Ȱȣ ���� ����)
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

	// ���� �ʱ�ȭ
	WSADATA wsaData;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	// Listen Socket ����
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET)
		err_quit("socket()");

	// �ּ� ����ü�� ���ε�
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

	// ��ſ� �ʿ��� ���� �غ�
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

		// ���� ���� ���
		printf("\n[TCP ����] Ŭ���̾�Ʈ ���� : %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

		while (true)
		{
			// ���� �ʱ�ȭ
			memset(&buf, 0, sizeof(buf));

			// ������ �ޱ�
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
				printf("\n[TCP ����] Ŭ���̾�Ʈ ���� : %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
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