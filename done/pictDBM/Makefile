CC = gcc
CFLAGS += -std=c99 -g
LDLIBS += -lssl -lcrypto
SRCS = db_utils.c pictDBM.c db_list.c error.c

pictDBM: $(SRCS)
	$(CC) $(LDLIBS) $(CFLAGS) -o pictDBM $(SRCS)

all: pictDBM

test: pictDBM
	./pictDBM list testDB01.pictdb_static > test_output.txt
	./pictDBM list testDB02.pictdb_static >> test_output.txt
	# -e only show differing parts -s tells if file equal -q tells if file differ
	diff -y -w test_output.txt correct_output.txt
	diff -s -w test_output.txt correct_output.txt
	diff -q -w test_output.txt correct_output.txt
