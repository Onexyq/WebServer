/*
 * webserver.h
 *
 *  	Created on: 2022-09-12
 *    Author: xueey
 */

#ifndef SERVER_WEBSERVER_H_
#define SERVER_WEBSERVER_H_

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>


class WebServer final {
public:
	WebServer();
	virtual ~WebServer();
	WebServer(WebServer &&other);
};

#endif /* SERVER_WEBSERVER_H_ */
