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

#include "../log/log.h"
#include "../pool/sqlconnpool.h"
#include "../pool/threadpool.h"
#include "../pool/sqlconnRAII.h"
#include "../http/httpconn.h"

class WebServer final {
public:
	WebServer(uint port, uint timeout, int optLinger);
	virtual ~WebServer();
	WebServer(WebServer &&other);
	void Start();

private:
	bool InitSocket();
	static int SetFdNonblock(int fd);

	bool optLinger_;
	bool isClose;
	bool asDaemon;
	uint port;
	uint timeout;  	//miliseconds
	int listenFd;
	char *srcDir;

	std::unique_ptr<ThreadPool> threadpool_;
	std::unordered_map<int, HttpConn> users_;

};

#endif /* SERVER_WEBSERVER_H_ */
