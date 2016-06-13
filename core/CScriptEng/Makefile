INCLUDE_OPTIONS = -I./ -I./include/ -I../../thirdparty/libxml2-2.9.2/include -I../ -I../../thirdparty/

TARGET=libCScriptEng.so

CPPS= \
	breakContinueSupport.cpp \
	BreakStatement.cpp \
	compile.cpp \
	ContinueStatement.cpp \
	DebugBreakStatement.cpp \
	DeclStatement.cpp \
	doWhileStatement.cpp \
	forStatement.cpp \
	functionStatement.cpp \
	FuncImpl.cpp \
	IfCondStatement.cpp \
	PureExpStatement.cpp \
	returnStatement.cpp \
	Statement.cpp \
	StatementBlock.cpp \
	SwitchStatement.cpp \
	varContainer.cpp \
	whileStatement.cpp \
	arrayObj.cpp \
	doCallContext.cpp \
	doubleObj.cpp \
	floatObj.cpp \
	int16Obj.cpp \
	int32Obj.cpp \
	int8Obj.cpp \
	objType.cpp \
	rtLib.cpp \
	stringObj.cpp \
	uint16Obj.cpp \
	uint32Obj.cpp \
	uint8Obj.cpp \
	vm.cpp \
	CScriptEng.cpp \
	Trace.cpp

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

COMPILE_CC=$(CROSS_COMPILE)cc
COMPILE_CXX=$(CROSS_COMPILE)g++

%.o: %.cpp
	$(COMPILE_CXX) $(INCLUDE_OPTIONS) $(CFLAGS) -o $@ $*.cpp
	
all: $(OBJS)
	$(COMPILE_CXX) -fPIC -shared -o $(TARGET) $(OBJS)

clean:
	@echo $(TARGET): Do cleaning...
	rm *.o -f
	rm *.so -rf