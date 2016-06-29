# mz02005@qq.com

# template
SolutionDir=../../

TargetDir=$(SolutionDir)Output/
TargetName=testCScript

TargetPath=$(TargetDir)$(TargetName)

INCLUDES=-I$(SolutionDir)core/ -I$(SolutionDir)thirdparty/ \
	-I$(SolutionDir)thirdparty/libxml2-2.9.2/include/

CPPS = \
	cRuntimeExt.cpp \
	testCScript.cpp \
	svnTool.cpp
	
OBJS=$(CPPS:.cpp=.o)

COMPILE_CC=$(CROSS_COMPILE)cc
COMPILE_CXX=$(CROSS_COMPILE)g++
COMPILE_AR=$(CROSS_COMPILE)ar
COMPILE_STRIP=$(CROSS_COMPILE)strip

CFLAGS=-O3 -fPIC -std=c++0x
LDFLAGS=-fPIC

%.o: %.cpp
	@echo Compiling $< ...
	@echo $(INCLUDES)
	$(COMPILE_CXX) $(INCLUDES) $(CFLAGS) -c -o $@ $*.cpp

all: OutputDir $(OBJS) $(TargetPath)

OutputDir:
	mkdir -v -p $(TargetDir)

$(TargetPath):
	@echo Linking ...
	@$(COMPILE_CXX) $(LDFLAGS) $(OBJS) -o $(TargetPath) -lCScriptEng -lnotstd -lxml2 -liconv -lzlib -ldl -lm -lpthread -lrt \
		-L../../thirdparty/libiconv/iconv/ \
		-L../../thirdparty/libzlib/zlib-1.2.8/ \
		-L../../thirdparty/libxml2-2.9.2/ \
		-L../../core/notstd/ \
		-L../../core/CScriptEng/
	@echo Stripping ...
	@$(COMPILE_STRIP) -g $(TargetPath)
	cp ../../thirdparty/libiconv/iconv/*.so $(TargetDir)
	cp ../../thirdparty/libzlib/zlib-1.2.8/*.so $(TargetDir)
	cp ../../thirdparty/libxml2-2.9.2/*.so $(TargetDir)
	cp ../../core/notstd/*.so $(TargetDir)
	cp ../../core/CScriptEng/*.so $(TargetDir)

clean:
	@echo Cleaning ...
	@rm -rf $(TargetDir)
	@rm -rf *.o
	@rm -rf *.so
