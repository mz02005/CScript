INCLUDE_OPTIONS = -I./

TARGET=libiconv.so

CS= \
	iconv.c \
	localcharset.c

OBJS=$(CS:.c=.o)

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
	
all: $(OBJS)
	@echo Generating archive ...
	$(COMPILE_CC) -fPIC -shared -o $(TARGET) $(OBJS)

clean:
	@echo $(ProjectName): Do cleaning...
	rm *.o -f
	rm *.so -rf