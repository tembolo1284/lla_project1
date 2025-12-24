# Employee Database (dbview)

A simple command-line employee database tool written in C. Stores employee records in a custom binary file format with network byte ordering for portability.

## Features

- Create new database files
- Add employee records (name, address, hours)
- List all employees
- Update existing employee records
- Delete employees by name
- Binary file format with magic number validation
- Network byte order for cross-platform compatibility

## Project Structure

```
lla_project1/
├── bin/
│   └── dbview          # Compiled binary
├── include/
│   ├── common.h        # Status codes
│   ├── file.h          # File operation declarations
│   └── parse.h         # Data structures and parsing declarations
├── obj/
│   ├── file.o
│   ├── main.o
│   └── parse.o
├── src/
│   ├── file.c          # File create/open operations
│   ├── main.c          # CLI parsing and dispatch
│   └── parse.c         # Header/employee read/write/modify
├── Makefile
└── README.md
```

## Building

```bash
make            # Build the binary
make clean      # Remove object files, binaries, and .db files
```

## Usage

### Command Line

```bash
./bin/dbview -f <database_file> [ACTION]
```

### Actions

| Flag | Description | Example |
|------|-------------|---------|
| `-n` | Create new database | `./bin/dbview -f mydb.db -n` |
| `-l` | List all employees | `./bin/dbview -f mydb.db -l` |
| `-a` | Add employee | `./bin/dbview -f mydb.db -a "John Doe,123 Main St,40"` |
| `-u` | Update employee | `./bin/dbview -f mydb.db -u "John Doe,456 Oak Ave,45"` |
| `-d` | Delete employee | `./bin/dbview -f mydb.db -d "John Doe"` |
| `-h` | Show help | `./bin/dbview -h` |

### Record Format

Records are specified as comma-separated values:

```
"Name,Address,Hours"
```

- **Name**: Employee name (max 255 characters) - used as lookup key for update/delete
- **Address**: Employee address (max 255 characters)
- **Hours**: Hours worked (unsigned integer)

## Makefile Targets

### Build Targets

| Target | Description |
|--------|-------------|
| `make` or `make default` | Build the dbview binary |
| `make clean` | Remove object files, binaries, and .db files |
| `make run` | Clean, build, create test db, and add sample record |

### Database Targets

All database targets use `DB=./mynewdb.db` by default. Override with `make <target> DB=./other.db`.

| Target | Description |
|--------|-------------|
| `make newdb` | Create a new empty database |
| `make list` | List all employees |
| `make add REC="..."` | Add an employee |
| `make update REC="..."` | Update an employee |
| `make delete NAME="..."` | Delete an employee |
| `make help` | Show all available targets |

### Examples

```bash
# Full build and test cycle
make run

# Create a fresh database
make newdb

# Add employees
make add REC="Alice Smith,100 First Ave,40"
make add REC="Bob Jones,200 Second St,35"

# List all employees
make list

# Update an employee's address and hours
make update REC="Alice Smith,300 Third Blvd,45"

# Delete an employee
make delete NAME="Bob Jones"

# Use a different database file
make list DB=./production.db
make add REC="New Hire,123 Work Lane,40" DB=./production.db
```

## Binary File Format

### Header (12 bytes)

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0 | 4 | magic | Magic number `0x4c4c4144` ("LLAD") |
| 4 | 2 | version | File format version (currently 1) |
| 6 | 2 | count | Number of employee records |
| 8 | 4 | filesize | Total file size in bytes |

### Employee Record (516 bytes each)

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0 | 256 | name | Employee name (null-terminated string) |
| 256 | 256 | address | Employee address (null-terminated string) |
| 512 | 4 | hours | Hours worked (unsigned int) |

All multi-byte integers are stored in network byte order (big-endian) for portability.

## Source Files

### main.c

- Command-line argument parsing using `getopt()`
- Action dispatch (create, list, add, update, delete)
- Error handling and cleanup

### file.c

- `create_db_file()` - Create new database file with read/write permissions
- `open_db_file()` - Open existing database file

### parse.c

- `create_db_header()` - Initialize new database header in memory
- `validate_db_header()` - Read and validate header from file
- `read_employees()` - Load employee records from file
- `output_file()` - Write header and employees to file
- `list_employees()` - Print formatted employee table
- `add_employee()` - Parse and add new employee record
- `update_employee()` - Find and update existing employee
- `delete_employee()` - Find and remove employee record

## Error Handling

The program returns:
- `0` (STATUS_SUCCESS) on success
- `-1` (STATUS_ERROR) on failure

Common errors:
- Missing `-f` flag (database file required)
- No action specified
- Multiple actions specified (only one allowed)
- Employee not found (for update/delete)
- Invalid database header (wrong magic number or version)
- File I/O errors

## Dependencies

- Standard C library
- POSIX extensions (`strdup`, `ftruncate`, `getopt`)
- Network functions (`htonl`, `htons`, `ntohl`, `ntohs` from `<arpa/inet.h>`)

Tested on Linux (Ubuntu). Should work on any POSIX-compliant system.

