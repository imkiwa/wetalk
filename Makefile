
export CC := gcc
export CFLAGS += -O2 -lsqlite3 -lpthread -lm -lreadline -lhistory -ltermcap -g

export MAKE := make --no-print-directory

export SRC := $(shell cd src && ls *.c)
export OBJ := $(SRC:.c=.o)
export BIN := wetalk



all:
	@cd src && $(MAKE)

.PHONY: clean
clean: 
	@-cd src && $(MAKE) clean
