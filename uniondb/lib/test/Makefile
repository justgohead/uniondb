#CFLAGS = -g -D _DEBUG -I../ -I/usr/local/include -L../ -L/usr/local/lib ../ztlib.a -pthread -lrt -ldl -Wall /usr/local/lib/libtcmalloc_minimal.a
#CFLAGS = -g -D _DEBUG -I../ -I/usr/local/include -L../ -L/usr/local/lib ../ztlib.a -pthread -ltcmalloc_minimal -lrt -ldl -Wall
CFLAGS =-I../ -I/usr/local/include -L../ -L/usr/local/lib ../leeloo.a -pthread -lrt -ldl -Wall
CC=g++
SRC=$(wildcard *.cpp)
OBJ=$(addprefix ./, $(addsuffix .o, $(basename $(SRC))))
TARGET=test

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC)  -o $@  $^ $(CFLAGS)

clean:
	rm -f $(TARGET) $(OBJ)


