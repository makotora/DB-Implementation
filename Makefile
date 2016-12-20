HT_OBJS = hash.o superfasthash.o HT_functions.o
EH_OBJS = exhash.o superfasthash.o EH_functions.o
BF = BF.o
SOURCE	= HT_test.c EH_test.c exhash.c hash.c superfasthash.c HT_functions.c EH_functions.c
HEADER  = hash.h exhash.h BF.h superfasthash.h HT_functions.h EH_functions.h
OUT  	= test

CC=gcc
DEBUG= -g
CFLAGS= -c -Wall $(DEBUG)
LFLAGS= -Wall $(DEBUG)

HT_test: $(HT_OBJS) HT_test.o
	$(CC) $(LFLAGS) $(HT_OBJS) $(BF) HT_test.o -o $(OUT)

EH_test: $(EH_OBJS) EH_test.o
	$(CC) $(LFLAGS) $(EH_OBJS) $(BF) EH_test.o -o $(OUT)

HT_main: $(HT_OBJS) main_example_HT.o
	$(CC) $(LFLAGS) main_example_HT.o hash.o superfasthash.o HT_functions.o $(BF) -o $(OUT)

EH_main: $(EH_OBJS) main_example_EH.o
	$(CC) $(LFLAGS) main_example_EH.o exhash.o superfasthash.o EH_functions.o $(BF) -o $(OUT)

main_example_HT.o: main_example_HT.c
	$(CC) $(CFLAGS) main_example_HT.c

main_example_EH.o: main_example_EH.c
	$(CC) $(CFLAGS) main_example_EH.c

HT_test.o: HT_test.c
	$(CC) $(CFLAGS) HT_test.c

EH_test.o: EH_test.c
	$(CC) $(CFLAGS) EH_test.c

hash.o: hash.c
	$(CC) $(CFLAGS) hash.c

exhash.o: exhash.c
	$(CC) $(CFLAGS) exhash.c

HT_functions.o: HT_functions.c
	$(CC) $(CFLAGS) HT_functions.c

EH_functions.o: EH_functions.c
	$(CC) $(CFLAGS) EH_functions.c

superfasthash.o: superfasthash.c
	$(CC) $(CFLAGS) superfasthash.c

clean:
	rm -f $(HT_OBJS) $(EH_OBJS) HT_test.o EH_test.o main_example_HT.o main_example_EH.o $(OUT)

count:
	wc $(SOURCE) $(HEADER)

