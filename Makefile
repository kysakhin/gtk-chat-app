CC = gcc
PKGCONFIG = `pkg-config --cflags --libs gtk4`
CFLAGS = -fsanitize=address
SRC = src/main.c src/ui.c
OBJ = $(SRC:.c=.o)
TARGET = chat_app

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(PKGCONFIG)

clean:
	rm -f $(TARGET) $(OBJ)

.PHONY: all clean
