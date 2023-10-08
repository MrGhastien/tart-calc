CFLAGS = -Wall -Wextra -I./include -g -Werror=return-type -fsanitize=address
SRC = ./src
HDR = ./include
OBJ = ./obj
BIN = ./bin

SRCS = $(wildcard $(SRC)/*.c) $(wildcard $(SRC)/**/*.c)
HDRS = $(wildcard $(HDR)/*.h) $(wildcard $(HDR)/**/*.h)

OBJS = $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCS))

SUBDIRS := $(foreach n,$(OBJS),$(dir $(n)))

TARGET = $(BIN)/tartiflum

.PHONY: clean all


all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN)/
	@echo -e "\e[93mLinking $@...\e[0m"
	gcc $(OBJS) $(CFLAGS) -o $@
	@echo -e "\e[92mFinished compiling!"

$(OBJ)/%.o: $(SRC)/%.c $(HDRS) | $(SUBDIRS)
	@echo -e "\e[33mCompiling $@...\e[0m"
	@gcc -c $< $(CFLAGS) -o $@

.SILENT:
$(BIN)/ $(OBJ)/:
	mkdir -p $@

$(OBJ)/%/ $(BIN)/%/:
	mkdir -p $@

clean:
	rm -fr bin
	rm -fr obj

