CC=gcc
CFLAGS=
OBJ     := ./obj
SRC     := ./src
SRCS    := $(wildcard $(SRC)/*.c)
OBJS    := $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCS))
EXE     := server

all: $(EXE)

$(OBJ)/%.o: $(SRC)/%.c header/server_process_function.h
	$(CC) $(CFLAGS) -c $< -o $@

server: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)


clean:
	rm $(EXE) $(OBJ)/*.o