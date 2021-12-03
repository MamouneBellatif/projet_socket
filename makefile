CC := gcc
SRC_DIR := src#repertoir source
OBJ_DIR := obj#repertoire fichiers objets
BIN_DIR := bin#repertoire executable
EXEC_SERVER := $(BIN_DIR)/server
EXEC_CLIENT := $(BIN_DIR)/client
CPPFLAGS := -Iinclude


all: $(EXEC_CLIENT) $(EXEC_SERVER) 

$(EXEC_CLIENT): $(OBJ_DIR)/client.o | $(BIN_DIR)
	$(CC) $< -o $@

$(EXEC_SERVER): $(OBJ_DIR)/server.o | $(BIN_DIR)
	$(CC) $< -o $@

$(OBJ_DIR)/client.o: $(SRC_DIR)/client.c | $(OBJ_DIR)
	$(CC) -c $< -o $@

$(OBJ_DIR)/server.o: $(SRC_DIR)/server.c | $(OBJ_DIR)
	$(CC) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	rm -rv $(OBJ_DIR)