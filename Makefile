CC = gcc
AS = nasm
CFLAGS = -Wall -Wextra -I./include -g -Werror=return-type
ASFLAGS = -felf64 -g
LDFLAGS =
LDLIBS = -lm

SRC = src
HDR = include
OBJ = obj
TST := tests

SRCS := $(wildcard $(SRC)/*.c) $(wildcard $(SRC)/**/*.c)
ASMS := $(wildcard $(SRC)/*.asm) $(wildcard $(SRC)/**/*.asm)
HDRS := $(wildcard $(HDR)/*.h) $(wildcard $(HDR)/**/*.h)

OBJS := $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCS))
OBJS += $(patsubst $(SRC)/%.asm,$(OBJ)/%.o,$(ASMS))

MAINS := main.c
MAINS := $(patsubst %.c,$(OBJ)/%.o,$(MAINS))

TSTS := $(wildcard $(TST)/*.c)
TST_OBJS := $(patsubst $(TST)/%.c,$(OBJ)/$(TST)/%.o,$(TSTS))

SUBDIRS := $(foreach n,$(OBJS),$(dir $(n)))

TARGET = tcalc
TEST_TARGET := tcalc-test

.PHONY: clean all check

all: CFLAGS += -fsanitize=address
all: LDFLAGS += -fsanitize=address
all: $(TARGET)

customalloc: LDLIBS += -L/home/mrghastien/epita/psc/malloc/malloc -lmalloc
customalloc: $(TARGET)

check: CFLAGS += -fsanitize=address
check: LDFLAGS += -fsanitize=address
check: LDLIBS += -lcriterion
check: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(filter-out $(MAINS),$(OBJS)) $(TST_OBJS)
	@printf "\e[93mLinking test executable $@...\e[0m\n"
	@$(CC) $(LDFLAGS) $(LDLIBS) -o $@ $^
	@printf "\e[92mFinished compiling!\e[0m\n"

$(TARGET): $(OBJS)
	@printf "\e[93mLinking executable $@...\e[0m\n"
	@$(CC) $(LDFLAGS) $(LDLIBS) -o $@ $^
	@printf "\e[92mFinished compiling!\e[0m\n"

$(OBJ)/$(TST)/%.o: $(TST)/%.c $(HDRS) | $(SUBDIRS)
	@echo -e "\e[33mCompiling test C source file $<...\e[0m"
	@$(CC) -c $(CFLAGS) -o $@ $<

$(OBJ)/%.o: $(SRC)/%.c $(HDRS) | $(SUBDIRS)
	@echo -e "\e[33mCompiling C source file $<...\e[0m"
	@$(CC) -c $(CFLAGS) -o $@ $<

$(OBJ)/%.o: $(SRC)/%.asm $(HDRS) | $(SUBDIRS)
	@echo -e "\e[33mCompiling Assembly file $<...\e[0m"
	@$(AS) $(ASFLAGS) -o $@ $<

$(OBJ)/%/:
	mkdir -p $@

clean:
	$(RM) $(OBJS) $(TST_OBJS)
	$(RM) $(TARGET) $(TEST_TARGET)
