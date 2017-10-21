# mz02005@qq.com

# template
SolutionDir=./

TargetDir=$(SolutionDir)Output/

DEP_LDTOOL=
ifeq ($(MACOS),y)
	DEP_LDTOOL = ldtool
endif

all: someHeader libiconv.so libxml2.so libzlib.so libnotstd.so libCScriptEng.so testCScript $(DEP_LDTOOL)
	cp $(SolutionDir)thirdparty/libzlib/zlib-1.2.8/libzlib.so $(TargetDir)
	cp $(SolutionDir)thirdparty/libxml2-2.9.2/libxml2.so $(TargetDir)
	cp $(SolutionDir)thirdparty/libiconv/iconv/libiconv.so $(TargetDir)
	cp $(SolutionDir)core/notstd/libnotstd.so $(TargetDir)
	cp $(SolutionDir)core/CScriptEng/libCScriptEng.so $(TargetDir)
	
ldtool:
	chmod +x ./ldtool
	./ldtool
	
someHeader:
	chmod +x ./getSVNVersion
	./getSVNVersion

testCScript: libCScriptEng.so
	make -C $(SolutionDir)test/testCScript

libCScriptEng.so: libnotstd.so
	make -C $(SolutionDir)core/CScriptEng

libnotstd.so: libxml2.so
	make -C $(SolutionDir)core/notstd

libiconv.so:
	make -C $(SolutionDir)thirdparty/libiconv/iconv/

libxml2.so: libiconv.so libzlib.so
	make -C $(SolutionDir)thirdparty/libxml2-2.9.2/

libzlib.so:
	make -C $(SolutionDir)thirdparty/libzlib/zlib-1.2.8/

OutputDir:
	mkdir -vp $(TargetDir)

clean:
	@echo Cleaning ...
	-make -C $(SolutionDir)core/notstd clean
	-make -C $(SolutionDir)thirdparty/libiconv/iconv/ clean
	-make -C $(SolutionDir)thirdparty/libxml2-2.9.2/ clean
	-make -C $(SolutionDir)thirdparty/libxml2-2.9.2/ clean
	-make -C $(SolutionDir)thirdparty/libzlib/zlib-1.2.8/ clean
	-make -C $(SolutionDie)test/testCScript clean
	-make -C $(SolutionDir)core/CScriptEng clean
