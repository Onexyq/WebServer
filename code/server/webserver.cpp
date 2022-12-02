/*
 *	@File: webserver.cpp
 *	@Author: xueey
 * @Date: 2022-09
 *	@copyleft Apache 2.0
 *
 */

#include "webserver.h"

using namespace std;


WebServer::WebServer(uint port = 80, uint timeout = 600000, int optLinger = false) :
		port(port), timeout(timeout), optLinger_(optLinger), isClose(false),
		asDaemon(false){
	// TODO Auto-generated constructor stub

}

WebServer::~WebServer() {
	close (listenFd);
	isClose = true;
	delete srcDir;
	SqlConnPool::Instance()->ClosePool();
}

WebServer::WebServer(WebServer &&other) {
	// TODO Auto-generated constructor stub

}

void WebServer::Start() {

}

/* Create listenFd */
bool WebServer::InitSocket() {
	int ret;
	struct sockaddr_in addr;
	if (port > 65535 || port < 1024) {
		LOG_ERROR("Port:%d error!", port);
		return false;
	}
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
	struct linger optLinger = { 0 };
	if (optLinger_) {
		optLinger.l_onoff = 1;
		optLinger.l_linger = 1;
	}

	listenFd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenFd < 0) {
		LOG_ERROR("Create socket error!", port);
		return false;
	}

	ret = setsockopt(listenFd, SOL_SOCKET, SO_LINGER, &optLinger,
			sizeof(optLinger));
	if (ret < 0) {
		close(listenFd);
		LOG_ERROR("Init linger error!", port);
		return false;
	}

	int optval = 1;
	/* Reuse Port  */
	ret = setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, (const void*) &optval,
			sizeof(int));
	if (ret == -1) {
		LOG_ERROR("setsockopt error!");
		close(listenFd);
		return false;
	}

	ret = bind(listenFd, (struct sockaddr*) &addr, sizeof(addr));
	if (ret < 0) {
		LOG_ERROR("Bind Port:%d error!", port);
		close(listenFd);
		return false;
	}

	ret = listen(listenFd, 6);
	if (ret < 0) {
		LOG_ERROR("Listen port:%d error!", port);
		close(listenFd);
		return false;
	}
	//ret = epoller_->AddFd(listenFd, listenEvent_ | EPOLLIN);
//	if (ret == 0) {
//		LOG_ERROR("Add listen error!");
//		close (listenFd);
//		return false;
//	}

	SetFdNonblock(listenFd);
	LOG_INFO("Server port:%d", port);
	return true;
}

int WebServer::SetFdNonblock(int fd) {
	assert(fd > 0);
	return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}
