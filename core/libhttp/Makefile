
SolutionDir=../../
TARGET=libhttp.so

INCLUDE_OPTIONS= \
	-I./ -I./include/ \
	-I$(SolutionDir)thirdparty/libxml2-2.9.2/include \
	-I../ -I../../thirdparty/ \
	-I$(SolutionDir)thirdparty/libzlib/zlib-1.2.8/ \
	-I$(SolutionDir)thirdparty/libevent/libevent-2.1.8-stable/include/

CPPS= \
	coder.cpp \
	dataBufferImpl.cpp \
	HttpRequest.cpp \
	HttpResponse.cpp \
	httpsdef.cpp \
	httpServerNew.cpp \
	libhttp.cpp

OBJS=$(CPPS:.cpp=.o)

CFLAGS=-c -DHAVE_ICONV
ifeq ($(ARM),y)
	CFLAGS += -DARM_PLATFORM
endif

ifeq ($(DEBUG),y)
	CFLAGS += -D_DEBUG -DDEBUG -g -fPIC -std=c++0x
else
	CFLAGS += -O3 -fPIC -std=c++11
endif

EXTLIB=
ifeq ($(MACOS),y)
	EXTLIB += -L../notstd/ -L../CScriptEng \
		-L$(SolutionDir)thirdparty/libevent/libevent-2.1.8-stable/.libs/ \
		-L$(SolutionDir)thirdparty/libxml2-2.9.2 \
		-L$(SolutionDir)thirdparty/libzlib/zlib-1.2.8 \
		-lnotstd -liconv -lzlib -lCScriptEng -lxml2 -lssl -lcrypto \
		$(SolutionDir)thirdparty/libevent/libevent-2.1.8-stable/.libs/libevent.a \
		$(SolutionDir)thirdparty/libevent/libevent-2.1.8-stable/.libs/libevent_openssl.a
endif

COMPILE_CC=$(CROSS_COMPILE)cc
COMPILE_CXX=$(CROSS_COMPILE)g++

%.o: %.cpp
	$(COMPILE_CXX) $(INCLUDE_OPTIONS) $(CFLAGS) -o $@ $*.cpp
	
all: testCScript
	$(COMPILE_CXX) -fPIC -shared -o $(TARGET) $(OBJS) $(EXTLIB)

libevent:
	cd $(SolutionDir)thirdparty/libevent/libevent-2.1.8-stable/ && chmod +x ./configure && (make || ./configure) && make && cd -
	#cd $(SolutionDir)thirdparty/libevent/libevent-2.1.8-stable/ && ./configure && cd -
	#make -C $(SolutionDir)thirdparty/libevent/libevent-2.1.8-stable

testCScript: libevent $(OBJS)
	make -C $(SolutionDir)

clean:
	@echo $(TARGET): Do cleaning...
	rm -rf *.o
	rm -rf *.so