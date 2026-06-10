# Compiler and Flags
CC = cc
CFLAGS = -Wall -Wextra -Werror -g
CRITERION_CFLAGS = $(shell pkg-config --cflags criterion 2>/dev/null)
CRITERION_LIBS = $(shell pkg-config --libs criterion 2>/dev/null)

# Archive and Flags
AR = ar
ARFLAGS = rcs

SRC_DIR = src
BUILD_DIR = build
MINISHELL_HEADER_DIR = include
TEST_DIR = test

# Source Files
SRC_FILES := $(shell find $(SRC_DIR) -type f -name '*.c')
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC_FILES))
TEST_SRC_FILES := $(filter-out $(SRC_DIR)/main.c, $(SRC_FILES))
TEST_OBJ_FILES := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(TEST_SRC_FILES))
TEST_FILES := $(shell find $(TEST_DIR) -type f -name '*.c')
TEST_OBJ := $(patsubst $(TEST_DIR)/%.c, $(BUILD_DIR)/test/%.o, $(TEST_FILES))

# Executable
NAME = minishell
TEST_NAME = minishell_tests

# Header Files
INCLUDES = -I${MINISHELL_HEADER_DIR}

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/test/%.o: $(TEST_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CRITERION_CFLAGS) $(INCLUDES) -c $< -o $@

# Default Target
all: $(NAME)

# Build Executable
$(NAME): $(OBJ_FILES)
	$(CC) $(CFLAGS) $(OBJ_FILES) -lreadline -o $(NAME)

$(TEST_NAME): $(TEST_OBJ_FILES) $(TEST_OBJ)
	$(CC) $(CFLAGS) $(TEST_OBJ_FILES) $(TEST_OBJ) $(CRITERION_LIBS) -lreadline -Wl,--wrap=malloc -o $(TEST_NAME)

test: $(TEST_NAME)
	./$(TEST_NAME)

clean:
	rm -rf $(BUILD_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re test
