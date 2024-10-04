.PHONY: clean all

CC = gcc
RM = rm -f
EXE = pcc
SRC = sources
INC = include
CFLAGS = -Wall -g -Wno-unused-function -Wno-unused-variable
SRCS = $(wildcard $(SRC)/*.c)
OBJS = $(patsubst $(SRC)/%.c, $(SRC)/%.o, $(SRCS))


all: $(EXE)

$(EXE): $(OBJS)
	$(CC) -g -Wall -o $@ $^

$(SRC)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -I$(INC) -c -o $@ $^

.depend: $(SRCS)
	$(CC) -MM -I$(INC) $(SRCS) > .depend

-include .depend

clean:
	$(RM) $(EXE) $(OBJS) .depend core
