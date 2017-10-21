SolutionDir=../../
TargetName=hmhttptest.so
TargetPath=$(SolutionDir)Output/$(TargetName)

INCLUDE_OPTIONS= \
	-I./ -I./include/ \
	-I$(SolutionDir)thirdparty/libxml2-2.9.2/include \
	-I$(SolutionDir)thirdparty/ \
	-I$(SolutionDir)thirdparty/libzlib/zlib-1.2.8/ \
	-I$(SolutionDir)core/ \
	-I$(SolutionDir)

TARGET=$(TargetPath)

CPPS= \
	conf.cpp \
	hmhttptest.cpp \
	MimeManager.cpp \
	PathIndex.cpp

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

EXTLIB = -L$(SolutionDir)core/notstd/ -lnotstd \
	-L$(SolutionDir)core/CScriptEng/ -lCScriptEng

COMPILE_CC=$(CROSS_COMPILE)cc
COMPILE_CXX=$(CROSS_COMPILE)g++

%.o: %.cpp
	$(COMPILE_CXX) $(INCLUDE_OPTIONS) $(CFLAGS) -o $@ $*.cpp
	
all: $(TargetPath)

$(TargetPath): $(OBJS)
	$(COMPILE_CXX) -fPIC -shared -o $(TargetPath) $(OBJS) $(EXTLIB)

clean:
	@echo $(TARGET): Do cleaning...
	rm *.o -f
	rm *.so -rf