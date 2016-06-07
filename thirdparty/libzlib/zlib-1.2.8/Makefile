INCLUDE_OPTIONS = -I./

TARGET=libzlib.so

minizipC= \
	contrib/minizip/ioapi.c \
	contrib/minizip/mztools.c \
	contrib/minizip/unzip.c \
	contrib/minizip/zip.c

zlibc= \
	adler32.c \
	compress.c \
	crc32.c \
	deflate.c \
	gzclose.c \
	gzlib.c \
	gzread.c \
	gzwrite.c \
	infback.c \
	inffast.c \
	inflate.c \
	inftrees.c \
	trees.c \
	uncompr.c \
	zutil.c

OBJSMINIZIP=$(minizipC:.c=.o)
OBJSZLIBC=$(zlibc:.c=.o)

CFLAGS=-c -DHAVE_ICONV
ifeq ($(ARM),y)
	CFLAGS += -DARM_PLATFORM
endif

ifeq ($(DEBUG),y)
	CFLAGS += -D_DEBUG -DDEBUG -g -fPIC
else
	CFLAGS += -O3 -fPIC
endif

COMPILE_CC=$(CROSS_COMPILE)cc
COMPILE_CXX=$(CROSS_COMPILE)g++

%.o: %.c
	$(COMPILE_CC) $(INCLUDE_OPTIONS) $(CFLAGS) -o $@ $*.c
	
all: $(OBJSMINIZIP) $(OBJSZLIBC)
	$(COMPILE_CC) -fPIC -shared -o $(TARGET) $(OBJSMINIZIP) $(OBJSZLIBC)

clean:
	@echo $(ProjectName): Do cleaning...
	rm *.o -f
	rm *.so -rf