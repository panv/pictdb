# C compiler
CC = gcc

# Compilation flags
CFLAGS += -std=c99 -Wall -Wextra --pedantic -g $$(pkg-config vips --cflags)

# Linking libraries and flags
LDLIBS += -lssl -lcrypto -lm $$(pkg-config vips --libs) -ljson-c

# Binary executables
TARGET = pictDBM
WEBTGT = pictDB_server

# Path to mongoose directory
MONGOOSEPATH = ./libmongoose6.2

# C source files
CMDSRCS = $(filter-out pictDB_server.c, $(wildcard *.c))
WEBSRCS = $(filter-out pictDBM.c db_gcollect.c db_create.c, $(wildcard *.c))

# C object files
CMDOBJS = $(CMDSRCS:.c=.o)
WEBOBJS = $(WEBSRCS:.c=.o)

all: $(TARGET) $(WEBTGT)

# Create command line executable
cli: $(TARGET)

# Create server executable
pictDB_server: $(WEBTGT)

$(TARGET): $(CMDOBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(CMDOBJS) $(LDLIBS)

$(WEBTGT): LDLIBS += -lmongoose
$(WEBTGT): LDFLAGS += -L $(MONGOOSEPATH)
$(WEBTGT): $(WEBOBJS)
# Build mongoose
	$(MAKE) -C $(MONGOOSEPATH)
# Build our executable
	$(CC) $(CFLAGS) -o $(WEBTGT) $(WEBOBJS) $(LDLIBS) $(LDFLAGS)

# Build .o's from .c's
$(WEBOBJS): CFLAGS += -I $(MONGOOSEPATH)
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

# Ignore format, clean, remake...
.PHONY: format clean test remake superclean remake_server

# Remove object files
clean:
	$(RM) *.o

# Remove object files and binaries
superclean: clean
	$(RM) $(WEBTGT) $(TARGET)

# Format with astyle
format:
	astyle --style=linux --indent=spaces=4 --convert-tabs --suffix=none \
	--add-brackets --unpad-paren --pad-oper --pad-header --align-pointer=type \
	--max-code-length=80 --lineend=linux -n *.c *.h

remake: superclean cli

remake_server: superclean pictDB_server

# to do: implement testing
#test:
#	$(CC) $(CFLAGS) -o vipstest tests/vipstest.c db_utils.c db_list.c error.c \
#	db_create.c db_delete.c image_content.c $(LDLIBS)
#	./vipstest testDB02.pictdb_dynamic pic1

