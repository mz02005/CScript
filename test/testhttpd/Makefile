# mz02005@qq.com

# template
SolutionDir=../../

TargetDir=$(SolutionDir)Output/
TargetName=testhttpd

TargetPath=$(TargetDir)$(TargetName)

INCLUDES=-I$(SolutionDir) -I$(SolutionDir)core/ -I$(SolutionDir)thirdparty/ \
	-I$(SolutionDir)thirdparty/libxml2-2.9.2/include/ \
	-I$(SolutionDir)thirdparty/libevent/libevent-2.1.8-stable/include/

CPPS = \
	testhttpd.cpp
	
OBJS=$(CPPS:.cpp=.o)

COMPILE_CC=$(CROSS_COMPILE)cc
COMPILE_CXX=$(CROSS_COMPILE)g++
COMPILE_AR=$(CROSS_COMPILE)ar
COMPILE_STRIP=$(CROSS_COMPILE)strip

CFLAGS=-O3 -fPIC -std=c++0x
LDFLAGS=-fPIC

LIBRT=
STRIP_FLAGS=
LIBEVENT_CPY=
ifneq ($(MACOS),y)
	LIBRT += -lrt -levent_openssl -levent
	STRIP_FLAGS += -g
	LIBEVENT_CPY += cp ../../thirdparty/libevent/libevent-2.1.8-stable/.libs/*.so $(TargetDir)
endif

%.o: %.cpp
	@echo Compiling $< ...
	@echo $(INCLUDES)
	$(COMPILE_CXX) $(INCLUDES) $(CFLAGS) -c -o $@ $*.cpp

all: $(OBJS) $(TargetPath)

libhttp:
	make -C $(SolutionDir)core/libhttp/

OutputDir:
	mkdir -v -p $(TargetDir)

$(TargetPath): OutputDir libhttp
	@echo Linking ...
	@$(COMPILE_CXX) $(LDFLAGS) $(OBJS) -o $(TargetPath) -lhttp -lCScriptEng -lnotstd -lxml2 -liconv -lzlib -ldl -lm -lpthread -lcrypto -lssl $(LIBRT) \
		-L../../thirdparty/libiconv/iconv/ \
		-L../../thirdparty/libzlib/zlib-1.2.8/ \
		-L../../thirdparty/libxml2-2.9.2/ \
		-L../../core/notstd/ \
		-L../../core/CScriptEng/ \
		-L../../core/libhttp/ \
		-L../../thirdparty/libevent/libevent-2.1.8-stable/.libs/
	@echo Stripping ...
	@$(COMPILE_STRIP) $(STRIP_FLAGS) $(TargetPath)
	cp ../../thirdparty/libevent/libevent-2.1.8-stable/.libs/*.so.* $(TargetDir)
	cp ../../thirdparty/libiconv/iconv/*.so $(TargetDir)
	cp ../../thirdparty/libzlib/zlib-1.2.8/*.so $(TargetDir)
	cp ../../thirdparty/libxml2-2.9.2/*.so $(TargetDir)
	cp ../../core/notstd/*.so $(TargetDir)
	cp ../../core/CScriptEng/*.so $(TargetDir)
	cp ../../core/libhttp/*.so $(TargetDir)
	$(LIBEVENT_CPY)

clean:
	@echo Cleaning ...
	@rm -rf $(TargetDir)
	@rm -rf *.o
	@rm -rf *.so
	make -C $(SolutionDir)core/libhttp/ clean
	make -C $(SolutionDir)thirdparty/libevent/libevent-2.1.8-stable clean
