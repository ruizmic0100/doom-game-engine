SRC_DIR = ./src
OBJS_DIR = ./obj
TARGET = $(OBJS_DIR)/app

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRC:$(SRC_DIR)/%.c=$(OBJS_DIR)/%.o)

CPPFLAGS = -I ./include -MMD -MP # preprocessor flags that optimize building if only header changed.
CFLAGS = -Wall -g             # Warnings all and debug info included in file.
LDFLAGS = -lGL -lGLU -lglut  # linker flags needed for opengl glut.

CC = g++

.PHONY: all clean

all: $(TARGET)
	$(info Finished!)

$(TARGET): $(SRC) | $(OBJS_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ $(LDFLAGS) -o $@.o

$(OBJS_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(OBJS_DIR) # The @ disables the output of the command.

-include $(OBJS:.o=.d)