.phony all:
all: rsi

rsi: rsi.c
	gcc rsi.c -lreadline -lhistory -o rsi
	gcc -o inf inf.c

.PHONY clean:
clean:
	-rm -rf *.o *.exe