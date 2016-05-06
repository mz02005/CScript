INCLUDE_OPTIONS = -I./

TARGET=libiconv.a

CS= \
	iconv.c \
	localcharset.c

OBJS=$(CS:.c=.o)

CFLAGS=-c -DHAVE_ICONV
ifeq ($(ARM),y)
	CFLAGS += -DARM_PLATFORM
endif

ifeq ($(DEBUG),y)
	CFLAGS += -D_DEBUG -DDEBUG -g
else
	CFLAGS += -O2
endif

COMPILE_CC=$(CROSS_COMPILE)cc
COMPILE_CXX=$(CROSS_COMPILE)g++
COMPILE_AR=$(CROSS_COMPILE)ar

%.o: %.c
	@echo Compiling $< ...
	$(COMPILE_CC) $(INCLUDE_OPTIONS) $(CFLAGS) -o $@ $*.c

all: $(OBJS)
	@echo Generating archive ...
	$(COMPILE_AR) -crv $(TARGET) $(OBJS)

clean:
	@echo $(ProjectName): Do cleaning...
	rm *.o -f
	rm *.a -rf