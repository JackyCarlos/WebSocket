CC = gcc
AR = ar

CFLAGS     = -Wall
DEBUGFLAGS = -DDEBUG_MODE -g

INC = -I ./include \
      -I ./sha1 \
      -I ./base64 \
      -I ./utils \
      -I ./http \
      -I ./utf8

LDFLAGS = -lpthread

SOURCES = wsserver.c \
          utf8/utf8.c \
          http/http.c \
          utils/utils.c \
          sha1/sha1.c \
          base64/base64.c

OBJFILES = wsserver.o \
           utf8/utf8.o \
           http/http.o \
           utils/utils.o \
           sha1/sha1.o \
           base64/base64.o

LIB = libwsserver.a

AUTOBahn_TARGET = testing/autobahn-server


all: $(LIB)


$(LIB): $(OBJFILES)
	$(AR) rcs $@ $^


autobahn-test:
	$(CC) $(INC) $(CFLAGS) $(DEBUGFLAGS) \
		testing/main.c $(SOURCES) \
		-o $(AUTOBahn_TARGET) \
		$(LDFLAGS)


wsserver.o: wsserver.c
	$(CC) $(INC) $(CFLAGS) -c $< -o $@

http/http.o: http/http.c
	$(CC) $(INC) $(CFLAGS) -c $< -o $@

utils/utils.o: utils/utils.c
	$(CC) $(INC) $(CFLAGS) -c $< -o $@

sha1/sha1.o: sha1/sha1.c
	$(CC) $(INC) $(CFLAGS) -c $< -o $@

base64/base64.o: base64/base64.c
	$(CC) $(INC) $(CFLAGS) -c $< -o $@

utf8/utf8.o: utf8/utf8.c
	$(CC) $(INC) $(CFLAGS) -c $< -o $@


clean:
	rm -f $(OBJFILES) \
	      $(LIB) \
	      $(AUTOBahn_TARGET)


.PHONY: all autobahn-test clean