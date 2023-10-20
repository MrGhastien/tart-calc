CC = gcc
AS = nasm
CFLAGS = -Wall -Wextra -I./include -g -Werror=return-type -fsanitize=address
ASFLAGS = -felf64 -g
LDFLAGS = -fsanitize=address
LDLIBS = -lm

SRC = ./src
HDR = ./include
OBJ = ./obj
BIN = ./bin

SRCS := $(wildcard $(SRC)/*.c) $(wildcard $(SRC)/**/*.c)
ASMS := $(wildcard $(SRC)/*.asm) $(wildcard $(SRC)/**/*.asm)
HDRS := $(wildcard $(HDR)/*.h) $(wildcard $(HDR)/**/*.h)

OBJS := $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCS))
OBJS += $(patsubst $(SRC)/%.asm,$(OBJ)/%.o,$(ASMS))

SUBDIRS := $(foreach n,$(OBJS),$(dir $(n)))

TARGET = $(BIN)/tartiflum

.PHONY: clean all


all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN)/
	@echo -e "\e[93mLinking Executable $@...\e[0m"
	@$(CC) $(LDFLAGS) $(LDLIBS) -o $@ $(OBJS)
	@echo -e "\e[92mFinished compiling!"

$(OBJ)/%.o: $(SRC)/%.c $(HDRS) | $(SUBDIRS)
	@echo -e "\e[33mCompiling C source file $<...\e[0m"
	@$(CC) -c $(CFLAGS) -o $@ $<

$(OBJ)/%.o: $(SRC)/%.asm $(HDRS) | $(SUBDIRS)
	@echo -e "\e[33mCompiling Assembly file $<...\e[0m"
	@$(AS) $(ASFLAGS) -o $@ $<

.SILENT:
$(BIN)/ $(OBJ)/:
	mkdir -p $@

$(OBJ)/%/ $(BIN)/%/:
	mkdir -p $@

clean:
	$(RM) $(OBJS)
	$(RM) $(TARGET)
