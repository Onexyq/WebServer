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

class WebServer final {
public:
	WebServer();
	WebServer(uint port, uint timeout, int optLinger);
	virtual ~WebServer();
	WebServer(WebServer &&other);

private:
	bool InitSocket();
	static int SetFdNonblock(int fd);

	uint port;
	bool m_optLinger;
	bool isClose;
	uint timeout;  	//miliseconds
	int listenFd;
	char *srcDir;

};

#endif /* SERVER_WEBSERVER_H_ */
