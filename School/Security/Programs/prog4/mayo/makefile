
all:	bigEnc  bigDec  Enc  Dec

bigEnc:	bigEnc.c
	gcc -g -o bigEnc bigEnc.c -lcrypto

bigDec:	bigDec.c
	gcc -g -o bigDec bigDec.c -lcrypto

Enc:	Enc.c
	gcc -g -o Enc Enc.c -lcrypto

Dec:	Dec.c
	gcc -g -o Dec Dec.c -lcrypto


clean:	
	rm -f bigEnc bigDec Enc Dec
