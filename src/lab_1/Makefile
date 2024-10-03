CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=c11

TESTS :=
TARGET := program
SOURCES := main.c

TESTSOURCESDIR := test_data
TESTRESULTDIR := test_result
BUILDDIR := build
TARGETDIR := .

OBJECTS := $(patsubst %.c,%.o,$(foreach file,$(SOURCES),$(file)))
TESTS := $(patsubst %.c,%,$(wildcard $(TESTSOURCESDIR)/*.c))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@$(CC) $(foreach object,$^,$(BUILDDIR)/$(notdir $(object))) -o $(TARGETDIR)/$@

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $(BUILDDIR)/$@

tests: $(TARGET) $(TESTS)

test%: test%.c
	@./$(TARGET) $(notdir $<)

clean:
	@rm $(TARGET)
	@rm $(BUILDDIR)/*
	@rm $(TESTRESULTDIR)/*