.PHONY: clean all

CC = gcc
RM = rm -f
EXE = pcc
SRC = sources
INC = include
CFLAGS = -Wall -g
SRCS = $(wildcard $(SRC)/*.c)
OBJS = $(patsubst $(SRC)/%.c, $(SRC)/%.o, $(SRCS))


all: $(EXE)

$(EXE): $(OBJS)
	$(CC) -g -Wall -o $@ $^

$(SRC)/%.o: $(SRC)/%.c
	$(CC) -g -Wall -I$(INC) -c -o $@ $^

.depend: $(SRCS)
	$(CC) -MM -I$(INC) $(SRCS) > .depend

-include .depend

clean:
	$(RM) $(EXE) $(OBJS) .depend core
