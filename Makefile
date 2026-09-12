# Makefile for CompiLearn Educational Compiler Construction Project
# Works seamlessly on MSYS2 UCRT64, MinGW, Linux, and macOS

CC ?= gcc
FLEX ?= flex
BISON ?= bison
CFLAGS ?= -Wall -Wextra -std=c11 -g -O2
LDFLAGS ?=

# Binary output name
ifeq ($(OS),Windows_NT)
    TARGET = compilearn.exe
    RM = rm -f
else
    TARGET = compilearn
    RM = rm -f
endif

# Source and Object files
OBJS = parser.tab.o \
       lex.yy.o \
       ast.o \
       symboltable.o \
       semantic.o \
       tac.o \
       optimizer.o \
       stackcode.o \
       menu.o \
       main.o

# Default target
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)
	@echo "=========================================="
	@echo " Build successful: $(TARGET)"
	@echo " Run ./$(TARGET) or ./$(TARGET) --help"
	@echo "=========================================="

# Bison rule
parser.tab.c parser.tab.h: parser.y ast.h
	$(BISON) -d parser.y

# Flex rule
lex.yy.c: lexer.l parser.tab.h ast.h
	$(FLEX) lexer.l

# Object compilation rules
parser.tab.o: parser.tab.c ast.h
	$(CC) $(CFLAGS) -c parser.tab.c -o $@

lex.yy.o: lex.yy.c parser.tab.h ast.h
	$(CC) $(CFLAGS) -c lex.yy.c -o $@

ast.o: ast.c ast.h
	$(CC) $(CFLAGS) -c ast.c -o $@

symboltable.o: symboltable.c symboltable.h ast.h
	$(CC) $(CFLAGS) -c symboltable.c -o $@

semantic.o: semantic.c semantic.h symboltable.h ast.h
	$(CC) $(CFLAGS) -c semantic.c -o $@

tac.o: tac.c tac.h ast.h
	$(CC) $(CFLAGS) -c tac.c -o $@

optimizer.o: optimizer.c optimizer.h tac.h
	$(CC) $(CFLAGS) -c optimizer.c -o $@

stackcode.o: stackcode.c stackcode.h tac.h
	$(CC) $(CFLAGS) -c stackcode.c -o $@

menu.o: menu.c menu.h ast.h symboltable.h semantic.h tac.h optimizer.h stackcode.h
	$(CC) $(CFLAGS) -c menu.c -o $@

main.o: main.c menu.h
	$(CC) $(CFLAGS) -c main.c -o $@

clean:
	$(RM) $(OBJS) parser.tab.c parser.tab.h lex.yy.c $(TARGET) compilearn compilearn.exe
	@echo "Cleaned all build artifacts."

# Web IDE and API convenience targets
run-web:
	python backend/app.py

test-api:
	python test_suite.py

.PHONY: all clean run-web test-api
