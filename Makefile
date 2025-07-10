CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
SRCDIR = src
OBJDIR = obj
BINDIR = bin

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Target executable
TARGET = $(BINDIR)/rust_compiler

# Default target
all: $(TARGET)

# Create directories
$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

# Build the executable
$(TARGET): $(OBJECTS) | $(BINDIR)
	$(CC) $(OBJECTS) -o $(TARGET)

# Compile source files
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(OBJDIR) $(BINDIR)

# Run the program
run: $(TARGET)
	./$(TARGET)

# Test the lexer
test: $(TARGET)
	./$(TARGET)

# Install (copy to /usr/local/bin)
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/

# Uninstall
uninstall:
	sudo rm -f /usr/local/bin/rust_compiler

# Show help
help:
	@echo "Available targets:"
	@echo "  all       - Build the project"
	@echo "  clean     - Remove build artifacts"
	@echo "  run       - Build and run the program"
	@echo "  test      - Build and test the lexer"
	@echo "  install   - Install to /usr/local/bin"
	@echo "  uninstall - Remove from /usr/local/bin"
	@echo "  help      - Show this help message"

.PHONY: all clean run test install uninstall help 