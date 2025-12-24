#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "file.h"
#include "parse.h"

typedef enum {
    ACT_NONE = 0,
    ACT_LIST,
    ACT_ADD,
    ACT_DEL,
    ACT_UPDATE,
    ACT_NEWFILE
} action_t;

void print_usage(char *argv[]) {
    printf("Usage: %s -f <database file> [OPTIONS]\n", argv[0]);
    printf("\nActions:\n");
    printf("  -n              Create new database file\n");
    printf("  -l              List all employees\n");
    printf("  -a <spec>       Add employee (e.g. \"name=John,address=123 Main,hours=40\")\n");
    printf("  -d <key>        Delete employee by key\n");
    printf("  -u <spec>       Update employee\n");
    printf("\nOther:\n");
    printf("  -h              Show this help\n");
}

static int set_action(action_t *dst, action_t a) {
    if (*dst != ACT_NONE) return -1;
    *dst = a;
    return 0;
}

int main(int argc, char *argv[]) {
    char *filepath = NULL;
    char *add_spec = NULL;
    char *del_key = NULL;
    char *upd_spec = NULL;
    action_t action = ACT_NONE;

    int c;
    opterr = 0;

    while ((c = getopt(argc, argv, "hf:nla:d:u:")) != -1) {
        switch (c) {
            case 'h':
                print_usage(argv);
                return STATUS_SUCCESS;

            case 'f':
                filepath = optarg;
                break;

            case 'n':
                if (set_action(&action, ACT_NEWFILE) != 0) {
                    fprintf(stderr, "Error: only one action allowed (-n/-l/-a/-d/-u)\n");
                    return STATUS_ERROR;
                }
                break;

            case 'l':
                if (set_action(&action, ACT_LIST) != 0) {
                    fprintf(stderr, "Error: only one action allowed (-n/-l/-a/-d/-u)\n");
                    return STATUS_ERROR;
                }
                break;

            case 'a':
                if (set_action(&action, ACT_ADD) != 0) {
                    fprintf(stderr, "Error: only one action allowed (-n/-l/-a/-d/-u)\n");
                    return STATUS_ERROR;
                }
                add_spec = optarg;
                break;

            case 'd':
                if (set_action(&action, ACT_DEL) != 0) {
                    fprintf(stderr, "Error: only one action allowed (-n/-l/-a/-d/-u)\n");
                    return STATUS_ERROR;
                }
                del_key = optarg;
                break;

            case 'u':
                if (set_action(&action, ACT_UPDATE) != 0) {
                    fprintf(stderr, "Error: only one action allowed (-n/-l/-a/-d/-u)\n");
                    return STATUS_ERROR;
                }
                upd_spec = optarg;
                break;

            case '?':
                if (optopt) {
                    fprintf(stderr, "Error: unknown option '-%c'\n", optopt);
                } else {
                    fprintf(stderr, "Error: unknown option\n");
                }
                print_usage(argv);
                return STATUS_ERROR;

            default:
                return STATUS_ERROR;
        }
    }

    if (optind < argc) {
        fprintf(stderr, "Error: unexpected argument: %s\n", argv[optind]);
        return STATUS_ERROR;
    }

    if (action == ACT_NONE) {
        fprintf(stderr, "Error: no action specified\n");
        print_usage(argv);
        return STATUS_ERROR;
    }

    if (filepath == NULL) {
        fprintf(stderr, "Error: missing -f <file>\n");
        return STATUS_ERROR;
    }

    int dbfd = -1;
    struct dbheader_t *dbhdr = NULL;
    struct employee_t *employees = NULL;

    switch (action) {
        case ACT_NEWFILE:
            dbfd = create_db_file(filepath);
            if (dbfd == STATUS_ERROR) {
                fprintf(stderr, "Error: failed to create database file\n");
                return STATUS_ERROR;
            }
            if (create_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
                fprintf(stderr, "Error: failed to create database header\n");
                return STATUS_ERROR;
            }
 
            // Write the header to disk BOOM!
            if (output_file(dbfd, dbhdr, NULL) == STATUS_ERROR) {
                fprintf(stderr, "Error: failed to write database header\n");
                return STATUS_ERROR;
            }
            printf("Created new database: %s\n", filepath);
            break;

        case ACT_LIST:
            dbfd = open_db_file(filepath);
            if (dbfd == STATUS_ERROR) {
                fprintf(stderr, "Error: failed to open database file\n");
                return STATUS_ERROR;
            }
            if (validate_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
                fprintf(stderr, "Error: invalid database header\n");
                return STATUS_ERROR;
            }
            if (read_employees(dbfd, dbhdr, &employees) == STATUS_ERROR) {
                fprintf(stderr, "Error: failed to read employees\n");
                return STATUS_ERROR;
            }
            list_employees(dbhdr, employees);
            break;

        case ACT_ADD:
            dbfd = open_db_file(filepath);
            if (dbfd == STATUS_ERROR) {
                fprintf(stderr, "Error: failed to open database file\n");
                return STATUS_ERROR;
            }
            if (validate_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
                fprintf(stderr, "Error: invalid database header\n");
                return STATUS_ERROR;
            }
            if (read_employees(dbfd, dbhdr, &employees) == STATUS_ERROR) {
                fprintf(stderr, "Error: failed to read employees\n");
                return STATUS_ERROR;
            }
            if (add_employee(dbhdr, employees, add_spec) == STATUS_ERROR) {
                fprintf(stderr, "Error: failed to add employee\n");
                return STATUS_ERROR;
            }
            if (output_file(dbfd, dbhdr, employees) == STATUS_ERROR) {
                fprintf(stderr, "Error: failed to write database\n");
                return STATUS_ERROR;
            }
            break;

        case ACT_DEL:
            // TODO: implement delete
            printf("Delete not yet implemented: %s\n", del_key);
            break;

        case ACT_UPDATE:
            // TODO: implement update
            printf("Update not yet implemented: %s\n", upd_spec);
            break;

        default:
            fprintf(stderr, "Error: unknown action\n");
            return STATUS_ERROR;
    }

    if (dbfd >= 0) {
        close(dbfd);
    }
    free(dbhdr);
    free(employees);

    return STATUS_SUCCESS;
}
