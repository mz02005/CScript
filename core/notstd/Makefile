INCLUDE_OPTIONS= \
	-I../ \
	-I./ -I./include/ -I./linuxbuild/ \
	-I../../thirdparty/libxml2-2.9.2/include \
	-I../../thirdparty/ -I../../thirdparty/libzlib/zlib-1.2.8/

TARGET=libnotstd.so

CPPS= \
	charconv.cpp \
	crc.cpp \
	filesystem.cpp \
	md5.cpp \
	notstd.cpp \
	nslist.cpp \
	objbase.cpp \
	simpleTool.cpp \
	stringHelper.cpp \
	vcverport.cpp \
	xmlparserHelper.cpp \
	xmlSerialize.cpp \
	zipWrapper.cpp

OBJS=$(CPPS:.cpp=.o)

CFLAGS=-c -DHAVE_ICONV
ifeq ($(ARM),y)
	CFLAGS += -DARM_PLATFORM
endif

ifeq ($(DEBUG),y)
	CFLAGS += -D_DEBUG -DDEBUG -g -fPIC -std=c++0x
else
	CFLAGS += -O3 -fPIC -std=c++0x
endif

COMPILE_CC=$(CROSS_COMPILE)cc
COMPILE_CXX=$(CROSS_COMPILE)g++

%.o: %.cpp
	$(COMPILE_CXX) $(INCLUDE_OPTIONS) $(CFLAGS) -o $@ $*.cpp
	
all: $(OBJS)
	echo $(OBJS)
	$(COMPILE_CXX) -fPIC -shared -o $(TARGET) $(OBJS)

clean:
	@echo $(TARGET): Do cleaning...
	rm *.o -f
	rm *.so -rf