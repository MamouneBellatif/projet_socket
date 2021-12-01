CC := gcc#compilateur
SRC_DIR := src#repertoir source
OBJ_DIR := obj#repertoire fichiers objets
BIN_DIR := bin#repertoire executable

EXEC := $(BIN_DIR)/server#executable
SRCS := $(wildcard $(SRC_DIR)/*.c)#sources fichier .c
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o) #fichiers objets a partir des fichiers sources

CPPFLAGS :=-Iinclude#include les header

.PHONY: all clean

all: $(EXEC) clean

$(EXEC): $(OBJS) | $(BIN_DIR)
	$(CC) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) -c $< -o $@
 	   
$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	$(RM) -rv $(OBJ_DIR)