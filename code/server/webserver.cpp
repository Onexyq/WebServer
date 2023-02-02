/*
 *	@File: webserver.cpp
 *	@Author: xueey
 *  @Date: 2022-09
 *	@copyleft Apache 2.0
 *
 */

#include "webserver.h"

using namespace std;


WebServer::WebServer(
				int port = 1166, int trigMode = 3, int timeout = 60000, bool optLinger = false,
				int sqlPort = 3306, const char* sqlUser = "root", const  char* sqlPwd = "root",
            	const char* dbName = "webserver", int connPoolNum = 12, int threadNum = 6,
				bool openLog = true, int logLevel = 1, int logQueSize = 1024
			) : 
			port(port), timeout(timeout), optLinger_(optLinger), isClose(false), asDaemon(false),
            timer_(new HeapTimer()), threadpool_(new ThreadPool(threadNum)), epoller_(new Epoller())
{

    srcDir = getcwd(nullptr, 256);
    assert(srcDir);
    
    char *path = (char*) malloc(256);
    char dir[256] = "/code/server";
    int len = strlen(srcDir) - strlen(dir);
    memcpy(path, srcDir, len);
    HttpConn::srcDir = path;
    strncat(path, "/resources/", 16);
    strncat(srcDir, "/resources/", 16);
    
    HttpConn::userCount = 0;
    
    SqlConnPool::Instance()->Init("localhost", sqlPort, sqlUser, sqlPwd, dbName, connPoolNum);

    InitEventMode(trigMode);
    if(!InitSocket()) { isClose = true;}

    if(openLog) {
        Log::Instance()->init(logLevel, "./log", ".log", logQueSize);
        if(isClose) { LOG_ERROR("========== Server init error!=========="); }
        else {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Port:%d, OpenLinger: %s", port, optLinger? "true":"false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                            (listenEvent & EPOLLET ? "ET": "LT"),
                            (connEvent & EPOLLET ? "ET": "LT"));
            LOG_INFO("LogSys level: %d", logLevel);
            LOG_INFO("srcDir: %s", HttpConn::srcDir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum, threadNum);
        }
    }

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

void WebServer::InitEventMode(int trigMode) {
    listenEvent = EPOLLRDHUP;
    connEvent = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigMode)
    {
    case 0:
        break;
    case 1:
        connEvent |= EPOLLET;
        break;
    case 2:
        listenEvent |= EPOLLET;
        break;
    case 3:
        listenEvent |= EPOLLET;
        connEvent |= EPOLLET;
        break;
    default:
        listenEvent |= EPOLLET;
        connEvent |= EPOLLET;
        break;
    }
    HttpConn::isET = (connEvent & EPOLLET);
}

void WebServer::Start() {
	int timeMS = -1; 
    if(!isClose) { LOG_INFO("========== Server start =========="); }
    while(!isClose) {
        if(timeout > 0) {
            timeMS = timer_->GetNextTick();
        }
        int eventCnt = epoller_->Wait(timeMS);
        for(int i = 0; i < eventCnt; i++) {
            int fd = epoller_->GetEventFd(i);
            uint32_t events = epoller_->GetEvents(i);
            if(fd == listenFd) {
                DealListen();
            }
            else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(users_.count(fd) > 0);
                CloseConn(&users_[fd]);
            }
            else if(events & EPOLLIN) {
                assert(users_.count(fd) > 0);
                DealRead(&users_[fd]);
            }
            else if(events & EPOLLOUT) {

                assert(users_.count(fd) > 0);
                DealWrite(&users_[fd]);
            } else {
                LOG_ERROR("Unexpected event");
            }
        }
    }
}

void WebServer::SendError(int fd, const char*info) {
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if(ret < 0) {
        LOG_WARN("send error to client[%d] error!", fd);
    }
    close(fd);
}

void WebServer::CloseConn(HttpConn* client) {
    assert(client);
    LOG_INFO("Client[%d] quit!", client->GetFd());
    epoller_->DelFd(client->GetFd());
    client->Close();
}

void WebServer::AddClient(int fd, sockaddr_in addr) {
    assert(fd > 0);
    users_[fd].init(fd, addr);
    if(timeout > 0) {
        timer_->add(fd, timeout, std::bind(&WebServer::CloseConn, this, &users_[fd]));
    }
    epoller_->AddFd(fd, EPOLLIN | connEvent);
    SetFdNonblock(fd);
    LOG_INFO("Client[%d] in!", users_[fd].GetFd());
}


void WebServer::DealListen() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(listenFd, (struct sockaddr *)&addr, &len);
        if(fd <= 0) { return;}
        else if(HttpConn::userCount >= MAX_FD) {
            SendError(fd, "Server busy!");
            LOG_WARN("Clients is full!");
            return;
        }
        AddClient(fd, addr);
    } while(listenEvent & EPOLLET);
}

void WebServer::DealRead(HttpConn* client) {
    assert(client);
    ExtentTime(client);
    threadpool_->AddTask(std::bind(&WebServer::OnRead, this, client));
}

void WebServer::DealWrite(HttpConn* client) {
    assert(client);
    ExtentTime(client);
    threadpool_->AddTask(std::bind(&WebServer::OnWrite, this, client));
}

void WebServer::ExtentTime(HttpConn* client) {
    assert(client);
    if(timeout > 0) { timer_->adjust(client->GetFd(), timeout); }
}

void WebServer::OnRead(HttpConn* client) {
    assert(client);
    int ret = -1;
    int readErrno = 0;
    ret = client->read(&readErrno);
    if(ret <= 0 && readErrno != EAGAIN) {
        CloseConn(client);
        return;
    }
    OnProcess(client);
}

void WebServer::OnProcess(HttpConn* client) {
    if(client->process()) {
        epoller_->ModFd(client->GetFd(), connEvent | EPOLLOUT);
    } else {
        epoller_->ModFd(client->GetFd(), connEvent | EPOLLIN);
    }
}

void WebServer::OnWrite(HttpConn* client) {
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->write(&writeErrno);
    if(client->ToWriteBytes() == 0) {
        if(client->IsKeepAlive()) {
            OnProcess(client);
            return;
        }
    }
    else if(ret < 0) {
        if(writeErrno == EAGAIN) {
            epoller_->ModFd(client->GetFd(), connEvent | EPOLLOUT);
            return;
        }
    }
    CloseConn(client);
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
	ret = epoller_->AddFd(listenFd, listenEvent | EPOLLIN);
	if (ret == 0) {
		LOG_ERROR("Add listen error!");
		close (listenFd);
		return false;
	}

	SetFdNonblock(listenFd);
	LOG_INFO("Server port:%d", port);
	return true;
}

int WebServer::SetFdNonblock(int fd) {
	assert(fd > 0);
	return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}


