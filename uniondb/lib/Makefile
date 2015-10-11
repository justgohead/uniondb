#CFLAGS = -D _DEBUG -static -I../../depend/loki -I/usr/local/include -L../../lib -L/usr/local/lib -g  -Wall
CFLAGS =-static -I../../depend/loki -I/usr/local/include -L/usr/local/lib -Wall
CC=g++
SRC=$(wildcard *.cpp)
OBJ=$(addprefix ./, $(addsuffix .o, $(basename $(SRC))))
TARGET=leeloo.a

all: $(TARGET)
 
$(TARGET): $(OBJ)
	$(AR) -rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(TARGET) $(OBJ)


