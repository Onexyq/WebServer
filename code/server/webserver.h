/*
 *	@File: webserver.h
 *	@Author: xueey
 * @Date: 2022-09
 *	@copyleft Apache 2.0
 *
 */

#ifndef SERVER_WEBSERVER_H_
#define SERVER_WEBSERVER_H_

#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "epoller.h"
#include "../log/log.h"
#include "../timer/heaptimer.h"
#include "../pool/sqlconnpool.h"
#include "../pool/threadpool.h"
#include "../pool/sqlconnRAII.h"
#include "../http/httpconn.h"

class WebServer final {
public:
	WebServer(
		int port, int trigMode, int timeout, bool optLinger,
		int sqlPort, const char* sqlUser, const  char* sqlPwd,
            const char* dbName, int connPoolNum, int threadNum,
			bool openLog, int logLevel, int logQueSize
		);
	virtual ~WebServer();
	WebServer(WebServer &&other);
	void Start();

private:
	bool InitSocket();
	void InitEventMode(int trigMode);
    void AddClient(int fd, sockaddr_in addr);
  
    void DealListen();
    void DealWrite(HttpConn* client);
    void DealRead(HttpConn* client);

    void SendError(int fd, const char*info);
    void ExtentTime(HttpConn* client);
    void CloseConn(HttpConn* client);

    void OnRead(HttpConn* client);
    void OnWrite(HttpConn* client);
    void OnProcess(HttpConn* client);

    static const int MAX_FD = 65536;

	static int SetFdNonblock(int fd);

	bool optLinger_;
	bool isClose;
	bool asDaemon;
	int port;
	int timeout;  	//miliseconds
	int listenFd;
	char *srcDir;

	uint32_t listenEvent;
    uint32_t connEvent;

 	std::unique_ptr<HeapTimer> timer_;
    std::unique_ptr<ThreadPool> threadpool_;
    std::unique_ptr<Epoller> epoller_;
    std::unordered_map<int, HttpConn> users_;

};

#endif /* SERVER_WEBSERVER_H_ */
