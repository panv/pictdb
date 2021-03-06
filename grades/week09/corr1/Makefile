# C compiler
CC = gcc
# flags
CFLAGS += -std=c99 -Wall -Wextra --pedantic -g $$(pkg-config vips --cflags)
# libraries
LDLIBS += -lssl -lcrypto -lm $$(pkg-config vips --libs)
# binary executable
TARGET = pictDBM
# C source files
SRCS = $(wildcard *.c)
# C object files, uses suffix replacement within a macro
# https://www.cs.swarthmore.edu/~newhall/unixhelp/howto_makefiles.html
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDLIBS)

# build .o's from .c's
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

# ignore format, clean and test
.PHONY: format clean test remake

# remove executable and object files
clean:
	$(RM) $(OBJS)

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
