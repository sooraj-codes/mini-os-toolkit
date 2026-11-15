CC     = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_DEFAULT_SOURCE
SRC    = main.c scheduling/scheduling.c deadlock/deadlock.c sync/sync.c
TARGET = mini-os-toolkit

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) -lpthread

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe
