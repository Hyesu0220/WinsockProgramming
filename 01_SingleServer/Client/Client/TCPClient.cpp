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
			// ���� ������� �ǹ�
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

	// ���� �ʱ�ȭ
	WSADATA wsaData;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
		err_quit("WSAStartup failed");

	// socket()
	SOCKET connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connectSocket == INVALID_SOCKET)
		err_quit("socket failed");

	// �ּ� ����ü �غ�
	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serverAddr.sin_port = htons(SERVERPORT);
	
	// connect()
	iResult = connect(connectSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (iResult == SOCKET_ERROR)
		err_quit("connect failed");

	// ������ ����� ���� ���� �غ�
	char buf[BUFSIZE + 1];
	int length = 0;

	printf("INVALID_SOCKET VALUE : %d\n", INVALID_SOCKET);

	while (true)
	{
		// ������ �Է� �ޱ�
		printf("\n[���� ������] ");
		if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
			break;

		// '\n' ����
		length = strlen(buf);
		if (buf[length - 1] == '\n')
			buf[length - 1] = '\0';
		if (strlen(buf) == 0)
			break;

		// �Է��� ������ ������
		iResult = send(connectSocket, buf, strlen(buf), 0);
		if (iResult == SOCKET_ERROR)
		{
			err_display("send failed.");
			closesocket(connectSocket);
			break;
		}

		printf("[TCP Ŭ���̾�Ʈ] %d ����Ʈ�� ���½��ϴ�.\n", iResult);
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

		// ���� ������ ���
		buf[iResult] = '\0';
		printf("[TCP Ŭ���̾�Ʈ] %d ����Ʈ�� �޾ҽ��ϴ�.\n", iResult);
		printf("[����������] %s\n", buf);
	}

	if (connectSocket != INVALID_SOCKET)
		closesocket(connectSocket);

	WSACleanup();
}