CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g 

TARGET = TestServer
OBJS = ../test/test.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $(TARGET)  -pthread -L/usr/lib64/mysql -lmysqlclient

clean:
	rm -rf ../bin/$(OBJS) $(TARGET)




