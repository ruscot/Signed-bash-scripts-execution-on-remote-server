CC=gcc
CFLAGS=
OBJ     := ./obj
SRC     := ./src
SRCS    := $(wildcard $(SRC)/*.c)
OBJS    := $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCS))
EXE     := client

all: $(EXE)

$(OBJ)/%.o: $(SRC)/%.c header/client_function.h
	$(CC) $(CFLAGS) -c $< -o $@

client: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)


clean:
	rm $(EXE) $(OBJ)/*.o