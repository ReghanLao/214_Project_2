CC = gcc
CFLAGS = -Wall

TARGET = outlier
SRC = outlier.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
