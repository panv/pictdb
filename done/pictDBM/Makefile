# C compiler
CC = gcc

# Compilation flags
CFLAGS += -std=c99 -Wall -Wextra --pedantic -g $$(pkg-config vips --cflags)

# Linking libraries and flags
LDLIBS += -lssl -lcrypto -lm $$(pkg-config vips --libs) -ljson-c

# binary executables
TARGET = pictDBM
WEBTGT = pictDB_server

# C source files
CMDSRCS = $(filter-out pictDB_server.c db_gcollect.c, $(wildcard *.c))
WEBSRCS = $(filter-out pictDBM.c db_gcollect.c db_create.c, $(wildcard *.c))

# C object files
CMDOBJS = $(CMDSRCS:.c=.o)
WEBOBJS = $(WEBSRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(CMDOBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(CMDOBJS) $(LDLIBS)

pictDB_server: $(WEBTGT)

$(WEBTGT): LDLIBS += -lmongoose
$(WEBTGT): LDFLAGS += -lmongoose
$(WEBTGT): LDFLAGS += -L ./libmongoose
$(WEBTGT): $(WEBOBJS)
	$(MAKE) -C ./libmongoose #Build mongoose
	$(CC) $(CFLAGS) -o $(WEBTGT) $(WEBOBJS) $(LDLIBS) $(LDFLAGS) #Build our executable

# build .o's from .c's
$(WEBOBJS): CFLAGS += -I ./libmongoose
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

# ignore format, clean, remake...
.PHONY: format clean test remake superclean remake_server

# remove object files
clean:
	$(RM) *.o

# remove object files and binaries
superclean: clean
	$(RM) $(WEBTGT) $(TARGET)

# format with astyle
format:
	astyle --style=linux --indent=spaces=4 --convert-tabs --suffix=none \
	--add-brackets --unpad-paren --pad-oper --pad-header --align-pointer=type \
	--max-code-length=80 --lineend=linux -n *.c *.h

remake: superclean all

remake_server: superclean pictDB_server

# to do: implement testing
#test:
#	$(CC) $(CFLAGS) -o vipstest tests/vipstest.c db_utils.c db_list.c error.c \
#	db_create.c db_delete.c image_content.c $(LDLIBS)
#	./vipstest testDB02.pictdb_dynamic pic1
