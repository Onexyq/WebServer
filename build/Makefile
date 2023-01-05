CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g 

TARGET = TinyWebServer
OBJS = ../code/buffer/*.cpp ../code/log/*.cpp ../code/timer/*.cpp \
	 ../code/pool/*.cpp ../code/http/*.cpp ../code/server/*.cpp\
       ../code/main.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ../bin/$(TARGET)  -pthread -L/usr/lib64/mysql -lmysqlclient

clean:
	rm -rf ../bin/$(OBJS) $(TARGET)




