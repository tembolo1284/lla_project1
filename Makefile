TARGET = bin/dbview
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))
DB = ./mynewdb.db

run: clean default
	./$(TARGET) -f ./mynewdb.db -n
	./$(TARGET) -f ./mynewdb.db -a "Timmy H.,123 Sheshire Ln.,120"

default: $(TARGET)

clean:
	rm -f obj/*.o
	rm -f bin/*
	rm -f *.db

$(TARGET): $(OBJ)
	gcc -o $@ $?

obj/%.o : src/%.c
	gcc -c $< -o $@ -Iinclude

list:
	./$(TARGET) -f $(DB) -l

help:
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@echo "  default   Build the dbview binary"
	@echo "  clean     Remove object files, binaries, and .db files"
	@echo "  run       Clean, build, then create a test db and add a record"
	@echo "  list      List all employees in the database"
	@echo "  help      Show this help message"
