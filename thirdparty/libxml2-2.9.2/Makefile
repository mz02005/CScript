INCLUDE_OPTIONS = -I./ -I./include/ -I../ -I./linuxbuild/

TARGET=libxml2.so

SC= \
	buf.c \
	c14n.c \
	catalog.c \
	chvalid.c \
	debugXML.c \
	dict.c \
	DOCBparser.c \
	encoding.c \
	entities.c \
	error.c \
	globals.c \
	hash.c \
	HTMLparser.c \
	HTMLtree.c \
	legacy.c \
	list.c \
	nanoftp.c \
	nanohttp.c \
	parser.c \
	parserInternals.c \
	pattern.c \
	relaxng.c \
	SAX.c \
	SAX2.c \
	schematron.c \
	threads.c \
	tree.c \
	uri.c \
	valid.c \
	xinclude.c \
	xlink.c \
	xmlcatalog.c \
	xmlIO.c \
	xmlmemory.c \
	xmlmodule.c \
	xmlreader.c \
	xmlregexp.c \
	xmlsave.c \
	xmlschemas.c \
	xmlschemastypes.c \
	xmlstring.c \
	xmlunicode.c \
	xmlwriter.c \
	xpath.c \
	xpointer.c

OBJS=$(SC:.c=.o)

CFLAGS=-c -DHAVE_ICONV
ifeq ($(ARM),y)
	CFLAGS += -DARM_PLATFORM
endif

ifeq ($(DEBUG),y)
	CFLAGS += -D_DEBUG -DDEBUG -g -fPIC -DLIBXML_THREAD_ENABLED
else
	CFLAGS += -O3 -fPIC -DLIBXML_THREAD_ENABLED
endif

COMPILE_CC=$(CROSS_COMPILE)cc
COMPILE_CXX=$(CROSS_COMPILE)g++

%.o: %.c
	$(COMPILE_CC) $(INCLUDE_OPTIONS) $(CFLAGS) -o $@ $*.c
	
all: $(OBJS)
	$(COMPILE_CC) -fPIC -shared -o $(TARGET) $(OBJS)

clean:
	@echo $(TARGET): Do cleaning...
	rm *.o -f
	rm *.so -rf