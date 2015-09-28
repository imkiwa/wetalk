$(BIN): $(OBJ)
	@$(CC) $(OBJ) $(CFLAGS) $(LDFLAGS) -o $(BIN)
	@echo "    CCLD  $(BIN)"


%.o: %.c
	@echo "    CC    $<"
	@$(CC) -I. -Iinclude -I../include $(CFLAGS) $(LDFLAGS) -c $< -o $@

.PHONY : clean
clean: 
	@echo "    CLEAN   " $(OBJ)
	@rm -rf $(OBJ) $(BIN)
