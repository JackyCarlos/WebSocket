CC          = gcc
CFLAGS      = -Wall
LDFLAGS     = -lpthread
OBJFILES    = main.o wsserver.o utf8/utf8.o http/http.o utils/utils.o sha1/sha1.o base64/base64.o
TARGET      = wsserver
INC         = -I ./include -I ./sha1 -I ./base64 -I ./utils -I ./http -I ./utf8
OBJDIR      = obj
SRCDIR      = src
DEBUGFLAGS  = -DDEBUG_MODE -g

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(INC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LDFLAGS)

debug: CFLAGS += $(DEBUGFLAGS)
debug: $(TARGET)

main.o: main.c 
	$(CC) $(INC) $(CFLAGS) -c main.c

http.o: http/http.c 
	$(CC) $(INC) $(CFLAGS) -c http/http.c

wsserver.o: wsserver.c 
	$(CC) $(INC) $(CFLAGS) -c wsserver.c

sha1.o: sha1/sha1.c 
	$(CC) $(INC) $(CFLAGS) -c sha1/sha1.c

base64.o: base64/base64.c 
	$(CC) $(INC) $(CFLAGS) -c base64/base64.c

utils.o: utils/utils.c 
	$(CC) $(INC) $(CFLAGS) -c utils/utils.c

utf8.o: utf8/utf8.c 
	$(CC) $(INC) $(CFLAGS) -c utf8/utf8.c

clean:
	rm -f $(OBJFILES) $(TARGET) *~
