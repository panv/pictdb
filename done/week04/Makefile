CC = gcc
CFLAGS += -std=c99 -g
LDLIBS += -lssl -lcrypto
SRCS = db_utils.c pictDBM.c db_list.c

pictDBM: $(SRCS)
	$(CC) $(LDLIBS) $(CFLAGS) -o pictDBM $(SRCS)

all: pictDBM
