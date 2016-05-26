# C compiler
CC = gcc
# flags
CFLAGS += -std=c99 -Wall -Wextra --pedantic -g $$(pkg-config vips --cflags)
# libraries
LDLIBS += -lssl -lcrypto -lm $$(pkg-config vips --libs) -ljson-c -lmongoose
LDFLAGS += -L ./libmongoose -I ./libmongoose
# binary executables
TARGET = pictDBM
WEBTGT = pictDB_server
# C source files
CMDSRCS = $(filter-out pictDB_server.c db_gcollect.c, $(wildcard *.c))
WEBSRCS = $(filter-out pictDBM.c db_gcollect.c, $(wildcard *.c))
# C object files
CMDOBJS = $(CMDSRCS:.c=.o)
WEBOBJS = $(WEBSRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(CMDOBJS)
	$(MAKE) -C ./libmongoose
	$(CC) $(CFLAGS) -o $(TARGET) $(CMDOBJS) $(LDLIBS) $(LDFLAGS)

pictDB_server: $(WEBTGT)

$(WEBTGT): $(WEBOBJS)
	$(MAKE) -C ./libmongoose
	$(CC) $(CFLAGS) -o $(WEBTGT) $(WEBOBJS) $(LDLIBS) $(LDFLAGS)

# build .o's from .c's
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

# ignore format, clean and test
.PHONY: format clean test remake

# remove object files
clean:
	$(RM) *.o

# format with astyle
format:
	astyle --style=linux --indent=spaces=4 --convert-tabs --suffix=none \
	--add-brackets --unpad-paren --pad-oper --pad-header --align-pointer=type \
	--max-code-length=80 --lineend=linux -n *.c *.h

remake:
	make clean
	$(RM) $(TARGET)
	make

# to do: implement testing
#test:
#	$(CC) $(CFLAGS) -o vipstest tests/vipstest.c db_utils.c db_list.c error.c \
#	db_create.c db_delete.c image_content.c $(LDLIBS)
#	./vipstest testDB02.pictdb_dynamic pic1
