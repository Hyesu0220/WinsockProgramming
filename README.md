# WinsockProgramming

## 개요
* 윈도우즈 소켓 프로그래밍을 다시 공부하면서 작성한 코드들을 Push 한다.

## 참고 사이트
* [WinSock2.h API](https://docs.microsoft.com/en-us/windows/win32/api/winsock2/)


## 목차

[1. 싱글 스레드 / 1대1 TCP 통신 ](#1-싱글-스레드--1대1-tcp-통신)

[2. 싱글 스레드 / 가변 길이 데이터 통신](#2-싱글-스레드--가변-길이-데이터-통신)

----
### 1. 싱글 스레드 / 1대1 TCP 통신
#### 서버
@see [MSDN 서버 예시 코드](https://docs.microsoft.com/en-us/windows/win32/winsock/complete-server-code)

**전체 적인 과정**
1. WSADATA 초기화
2. listen socket 생성
3. 서버의 주소 정보가 담긴 SOCKADDR 준비
4. listen socket 과 SOCKADDR 바인드
5. 바인딩이 완료된 listen socket을 listening 상태로 만든다.
6. accept()로 클라이언트의 connect()를 받는다.
7. 송·수신을 한다.
> recv(), send()
8. 연결이 종료된 클라이언트 소켓은 closesocket()으로 완전히 닫아준다.
> shutdown() 함수와 closesocket() 함수의 차이도 알아두자.
9. WSACleanup()으로 WSADATA를 정리한다.
#### 클라이언트
@see [MSDN 클라이언트 예시 코드](https://docs.microsoft.com/en-us/windows/win32/winsock/complete-client-code)

**전체 적인 과정**
1. WSADATA 초기화
2. 서버와 통신용 socket 생성
3. 서버의 주소 정보가 담긴 SOCKADDR 준비
4. 서버의 socket과 SOCKADDR로 connect() 요청한다.
5. connect()가 정상적으로 이루어졌다면 서버와 데이터 송·수신을 한다.
> recv(), send()
6. 연결이 종료된 클라이언트 소켓은 closesocket()으로 완전히 닫아준다.
7. WSACleanup()으로 WSADATA를 정리한다.

----
### 2. 싱글 스레드 / 가변 길이 데이터 통신
#### 서버 통신 과정
1. WSADATA 초기화
2. listen socket 생성
3. 서버의 주소 정보가 담긴 SOCKADDR 준비
4. listen socket 과 SOCKADDR 바인드
5. 바인딩이 완료된 listen socket을 listening 상태로 만든다.
6. accept()로 클라이언트의 connect()를 받는다.
7. int(4byte)를 받는다. ( 데이터 크기 수신 )
8. 7에서 받은 크기 만큼 받는다. ( 데이터 수신 )
9. 연결이 종료된 클라이언트 소켓은 closesocket()으로 완전히 닫아준다.
10. WSACleanup()으로 WSADATA를 정리한다.

#### 클라이언트 통신 과정
1. WSADATA 초기화
2. 서버와 통신용 socket 생성
3. 서버의 주소 정보가 담긴 SOCKADDR 준비
4. 서버의 socket과 SOCKADDR로 connect() 요청한다.
5. 사용자에게 보낼 데이터를 입력받는다.
6. int(4byte)를 보낸다. ( 데이터 크기 송신 )
7. 6에서 보낸 int 값 만큼 만큼 보낸다. ( 데이터 송신 )
8. 연결이 종료된 클라이언트 소켓은 closesocket()으로 완전히 닫아준다.
9. WSACleanup()으로 WSADATA를 정리한다.