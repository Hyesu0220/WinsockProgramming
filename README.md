# WinsockProgramming

## 개요
* 윈도우즈 소켓 프로그래밍을 다시 공부하면서 작성한 코드들을 Push 한다.

## 참고 사이트
* [WinSock2.h API](https://docs.microsoft.com/en-us/windows/win32/api/winsock2/)


## 목차
1. [싱글 스레드 / 1대1 TCP 통신 ](#1-싱글-스레드--1대1-tcp-통신)
2. [싱글 스레드 / 가변 길이 데이터 통신](#2-싱글-스레드--가변-길이-데이터-통신)
3. [스레드](#3-스레드)

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

----
### 3. 스레드
#### std::thread
- [Thread Reference](http://www.cplusplus.com/reference/thread/thread/)
- [Thread Priority 구현 예시](https://stackoverflow.com/questions/18884510/portable-way-of-setting-stdthread-priority-in-c11)

----
### 4. 스레드 Lock / Unlock
#### std::mutex
- [Reference](https://www.cplusplus.com/reference/mutex/mutex/?kw=mutex)
- [Vector에 thread 클래스 push하기](https://stackoverflow.com/questions/30768216/c-stdvector-of-independent-stdthreads)
- [Thread 생성자 설명](http://www.cplusplus.com/reference/thread/thread/thread/)
#### std::lock
- 한 번에 여러 mutex의 lock()을 할 때 사용한다.
- 둘 이상의 mutex를 사용할 때 `std::mutex.lock()` 호출로 인한 데드락 발생을 방지하기 위해 사용하는 함수이다.
- [Reference](http://www.cplusplus.com/reference/mutex/lock/)
#### std::lock_guard
- 생성자에서 Lock, 소멸자에서 Unlock 된다.
- 함수 Call 종료시 소멸된다.
- [Reference](http://www.cplusplus.com/reference/mutex/lock_guard/)
#### std::unique_lock
- std::lock_guard와 동일하게 생성시 lock되고 소멸시 unlock되지만 옵션을 통해 생성시 lock을 시키지 않고 원하는 타이밍에 lock 시킬 수 있다.
- **Lock 옵션 종류**
  - ***std::defer_lock*** : 기본적으로 lock이 걸리지 않으며 잠금 구조만 생성된다. `lock()` 함수를 호출 될 때 잠금이 된다. 둘 이상의 뮤텍스를 사용하는 상황에서 데드락이 발생 할 수 있습니다.(std::lock을 사용한다면 해결 가능합니다.)
  - ***std::try_to_lock*** : 기본적으로 lock이 걸리지 않으며 잠금 구조만 생성된다. 내부적으로 `try_lock()`을 호출해 소유권을 가져오며 실패하더라도 바로 false를 반환 합니다. `lock.owns_lock()` 등의 코드로 자신이 락을 걸 수 있는 지 확인이 필요하다.
  - ***std::adopt_lock*** : 기본적으로 lock이 걸리지 않으며 잠금 구조만 생성된다. 현재 호출 된 쓰레드가 뮤텍스의 소유권을 가지고 있다고 가정한다. 즉, 사용하려는 mutex 객체가 이미 lock 되어 있는 상태여야 한다. (이미 lock 된 후 unlock을 하지않더라도 unique_lock 으로 생성 해 unlock해줄수 있다.)
#### std::mutex를 이용한 Semaphore
- Semaphore는 std에서는 존재하지 않아 따로 구현해주어야한다.
- Semaphore의 목적은 공유 자원에 정해진 갯수의 프로세스가 접근할 수 있다.
- 접근할 때 Count가 감소시키고 벗어날 떄 Count를 증가시키며 Count가 0일 때는 접근을 차단시킨다.
- Semaphore 자체도 공유 자원이기 때문에 내부적으로 mutex로 lock과 unlock을 이용하여 구현한다.
- Semaphore의 동시 접근 가능 프로세스 수가 1인 경우에는 Event와 비슷하다.