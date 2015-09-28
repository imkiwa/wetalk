export CC := gcc
export CFLAGS += -O2  -lm -ldl -lpthread -lsqlite3 -lreadline -lhistory -ltermcap -lncursesw -fPIC --pie -g
export LDFLAGS += 
export MAKE := make --no-print-directory

export SRC := $(shell cd src && ls *.c)
export OBJ := $(SRC:.c=.o)
export BIN := wetalk



all:
	@cd src && $(MAKE)

.PHONY: clean
clean: 
	@-cd src && $(MAKE) clean
