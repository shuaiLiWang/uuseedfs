#Makefile
#author: yinhaichen<yinhaichen@ceopen.cn>
#date  : 2009-06-10

CC = gcc
C++ = g++
LINK = g++

COMPILER_FLAGS = -D_DEBUG #-D_REENTRANT -D__USE_POSIX -D__linux__ -pipe -fPIC  


#CORE_LINK_LIBS = -lpthread -lreadline -lncurses -ldl -lnsl -lresolv -lm -lcrypt -lstdc++
CORE_LINK_LIBS =  -lpthread -ldl -lnsl -lresolv -lm -lcrypt -lstdc++ 

CCFLAGS = $(COMPILER_FLAGS) -g -c -m32 -O0
C++FLAGS = $(COMPILER_FLAGS) -g -c -m32 -O0

A_COMPILE  = ar -rc
SO_COMPILE = $(C++) -rdynamic -fPIC -shared

LIBS = $(CORE_LINK_LIBS)

#***********************INPUT**************************
TARGET = uusee_dfs

TEMPPATH = 

LIBS += 
INCLUDES += -I.\
			-I./public \
			-I./util

C++FILES =	main.cpp \
			md5.cpp \
			dfsmanager.cpp \
			dhtdataset.cpp \
			filenotify.cpp \
			peerlist.cpp \
			udpsocket.cpp \
			commonstruct.cpp \
			Ini.cpp \
			Log.cpp \
			MutexObj.cpp \
			dfsprotocol.cpp \
			FileSystemWatcher.cpp \
			ThreadObj.cpp \
 			CallbackFuncObj.cpp \
 			statistic.cpp \
			crc32.cpp

CFILES =  ./util/log_agent.c \
		  ./util/log_debug.c \
		  ./util/log_agent_util.c \
		  ./public/gen_dir.c

#***********************INPUT**************************

OBJFILE = $(CFILES:.c=.o) $(C++FILES:.cpp=.o)

all:$(TARGET)

$(TARGET): $(OBJFILE)
	$(C++) -o $@ $^ $(LIBS) $(SOPATH) -m32 

%.o:%.c
	$(CC) -o $@ $(CCFLAGS) $< $(INCLUDES)

%.o:%.cpp
	$(C++) -o $@ $(C++FLAGS) $< $(INCLUDES)

clean:
	rm -rf $(TARGET)
	rm -rf $(OBJFILE)
install:
	mkdir -p $(TEMPPATH)
	mv -f $(OBJFILE) $(TEMPPATH)
