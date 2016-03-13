CC = gcc
CFLAGS += -std=c99 -g
LDLIBS += -lssl -lcrypto
SRCS = db_utils.c pictDBM.c db_list.c

pictDBM: $(SRCS)
	$(CC) $(LDLIBS) $(CFLAGS) -o pictDBM $(SRCS)

all: pictDBM

test: pictDBM
	./pictDBM list testDB01.pictdb_static > test_output.txt
	./pictDBM list testDB02.pictdb_static >> test_output.txt
	#the two outputs side by side:
	#diff -y test_output.txt correct_output.txt
	#just tells if outputs are equal:
	diff -q test_output.txt correct_output.txt
