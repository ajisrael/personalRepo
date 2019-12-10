all: batchEnc

batchEnc:  encryptWithPhrase.o batchEnc.o
	gcc -o batchEnc batchEnc.o encryptWithPhrase.o -lcrypto

batchEnc.o: batchEnc.c encryptWithPhrase.h
	gcc -c batchEnc.c

encryptWithPhrase.o: encryptWithPhrase.c encryptWithPhrase.h
	gcc -c encryptWithPhrase.c

clean:
	rm encryptWithPhrase.o batchEnc.o batchEnc

