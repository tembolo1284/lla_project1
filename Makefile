TARGET = bin/dbview
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))
DB = ./mynewdb.db

run: clean default
	./$(TARGET) -f $(DB) -n
	./$(TARGET) -f $(DB) -a "Timmy H.,123 Sheshire Ln.,120"

default: $(TARGET)

clean:
	rm -f obj/*.o
	rm -f bin/*
	rm -f *.db

$(TARGET): $(OBJ)
	gcc -o $@ $?

obj/%.o : src/%.c
	gcc -c $< -o $@ -Iinclude

# Database operations
newdb:
	./$(TARGET) -f $(DB) -n

list:
	./$(TARGET) -f $(DB) -l

add:
	@test -n "$(REC)" || (echo "Usage: make add REC=\"Name,Address,Hours\"" && exit 1)
	./$(TARGET) -f $(DB) -a "$(REC)"

update:
	@test -n "$(REC)" || (echo "Usage: make update REC=\"Name,NewAddress,NewHours\"" && exit 1)
	./$(TARGET) -f $(DB) -u "$(REC)"

delete:
	@test -n "$(NAME)" || (echo "Usage: make delete NAME=\"Employee Name\"" && exit 1)
	./$(TARGET) -f $(DB) -d "$(NAME)"

help:
	@echo "Usage: make [target]"
	@echo ""
	@echo "Build Targets:"
	@echo "  default      Build the dbview binary"
	@echo "  clean        Remove object files, binaries, and .db files"
	@echo "  run          Clean, build, create test db, and add sample record"
	@echo ""
	@echo "Database Targets (uses DB=$(DB)):"
	@echo "  newdb        Create a new empty database"
	@echo "  list         List all employees"
	@echo "  add          Add employee"
	@echo "  update       Update employee (lookup by name, change address/hours)"
	@echo "  delete       Delete employee by name"
	@echo ""
	@echo "Examples:"
	@echo "  make run                                    # Build and create test db"
	@echo "  make newdb                                  # Create fresh database"
	@echo "  make add REC=\"John Smith,789 Pine St,40\"    # Add new employee"
	@echo "  make list                                   # Show all employees"
	@echo "  make update REC=\"John Smith,999 Oak Ave,50\" # Update John's info"
	@echo "  make delete NAME=\"John Smith\"               # Remove John"
	@echo ""
	@echo "To use a different database:"
	@echo "  make list DB=./other.db"
