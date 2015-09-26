$(BIN): $(OBJ)
	@$(CC) $(OBJ) $(CFLAGS) -o $(BIN)
	@echo "    CCLD  $(BIN)"


%.o: %.c
	@echo "    CC    $<"
	@$(CC) -I. -Iinclude -I../include $(CFLAGS) -c $< -o $@


.PHONY : clean
clean: 
	@echo " CLEAN   " $(OBJ)
	@rm -rf $(OBJ) $(BIN)
