# all: 
# 	@if [ "`uname -s`" = "Linux" ] || [ "`uname -s`" = "Darwin" ]; then \
# 		make -f Makefile.linux ; \
# 	else \
# 		make -f Makefile.sunos ; \
# 	fi

# clean: 
# 	@if [ "`uname -s`" = "Linux" ] || [ "`uname -s`" = "Darwin" ]; then \
# 		make clean -f Makefile.linux ; \
# 	else \
# 		make clean -f Makefile.sunos ; \
# 	fi
CC=g++
CFLAGS=-Wall -g -O2 
LIBS=-lcrypto -lssl -pthread

all: myftpclient myftpserver

myftpclient: myftpclient.c
	$(CC) $(CFLAGS) myftpclient.c -o $@ $(LIBS)

myftpserver: myftpserver.c
	$(CC) $(CFLAGS) myftpserver.c -o $@ $(LIBS)

clean:
	@rm -f myftpclient
	@rm -f myftpserver