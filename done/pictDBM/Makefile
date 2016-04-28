CC = gcc
CFLAGS += -std=c99 -Wall -g $$(pkg-config vips --cflags)
LDLIBS += -lssl -lcrypto $$(pkg-config vips --libs) #-lm?
TARGET = pictDBM
SRCS = db_utils.c pictDBM.c db_list.c error.c db_create.c \
       db_delete.c image_content.c pictDBM_tools.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDLIBS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)


clean:
	$(RM) $(TARGET) *.o

format: $(SRCS)
	astyle --style=linux --indent=spaces=4 --convert-tabs --suffix=none \
	--add-brackets --unpad-paren --pad-oper --pad-header --align-pointer=type \
	--max-code-length=80 --lineend=linux -n *.c *.h
