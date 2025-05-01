CC=gcc
FLAGS=-MMD -MP
TARGET=app

SRC_DIR=src
OBJ_DIR=obj
DEPS_DIR=deps

SRC_FILES=$(wildcard $(SRC_DIR)/*.c)
OBJ_FILES=$(SRC_FILES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS_FILES=$(OBJ_FILES:$(OBJ_DIR)/%.o=$(DEPS_DIR)/%.d)

$(shell mkdir -p $(OBJ_DIR) $(DEPS_DIR))

all: $(TARGET)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(FLAGS) -c $< -o $@ -MF $(DEPS_DIR)/$*.d
	
$(TARGET): $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $(TARGET)
	
-include $(DEPS_FILES)
