
export CC := gcc
export CFLAGS += -O2 -lsqlite3 -lpthread -lm

export MAKE := make --no-print-directory

export SRC := $(shell cd src && ls *.c)
export OBJ := $(SRC:.c=.o)
export BIN := wetalk



all:
	@cd src && $(MAKE)

wetalk-test: wetalk-test.o
	$(CC) -o $@ $< -lreadline -ltermcap

.PHONY: clean
clean: 
	@-cd src && $(MAKE) clean
	@$(RM) wetalk-test.o wetalk-test
