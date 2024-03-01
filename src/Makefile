CC			= gcc
CFLAGS		=
LDFLAGS		= 
OBJFILES	= main.o wsserver.o utils/utils.o http.o sha1/sha1.o 
TARGET		= wsserver
INC			= -I include 
OBJDIR		= obj
SRCDIR		= src

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(INC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LDFLAGS)

clean:
	rm -f $(OBJFILES) $(TARGET) *~
