#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "common.h"
#include "parse.h"

int create_db_header(int fd, struct dbheader_t **headerOut) {
    struct dbheader_t *hdr = calloc(1, sizeof(struct dbheader_t));
    if (hdr == NULL) {
        perror("calloc");
        return STATUS_ERROR;
    }

    hdr->magic = HEADER_MAGIC;
    hdr->version = 1;
    hdr->count = 0;
    hdr->filesize = sizeof(struct dbheader_t);

    *headerOut = hdr;
    return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct dbheader_t **headerOut) {
    if (fd < 0) {
        fprintf(stderr, "Invalid file descriptor\n");
        return STATUS_ERROR;
    }

    struct dbheader_t *hdr = calloc(1, sizeof(struct dbheader_t));
    if (hdr == NULL) {
        perror("calloc");
        return STATUS_ERROR;
    }

    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("lseek");
        free(hdr);
        return STATUS_ERROR;
    }

    ssize_t bytes = read(fd, hdr, sizeof(struct dbheader_t));
    if (bytes != sizeof(struct dbheader_t)) {
        fprintf(stderr, "Failed to read header (got %zd bytes)\n", bytes);
        free(hdr);
        return STATUS_ERROR;
    }

    // Convert from network byte order
    hdr->magic = ntohl(hdr->magic);
    hdr->filesize = ntohl(hdr->filesize);
    hdr->count = ntohs(hdr->count);
    hdr->version = ntohs(hdr->version);

    if (hdr->magic != HEADER_MAGIC) {
        fprintf(stderr, "Invalid magic number: 0x%08x (expected 0x%08x)\n",
                hdr->magic, HEADER_MAGIC);
        free(hdr);
        return STATUS_ERROR;
    }

    if (hdr->version != 1) {
        fprintf(stderr, "Unsupported version: %d\n", hdr->version);
        free(hdr);
        return STATUS_ERROR;
    }

    *headerOut = hdr;
    return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
    if (fd < 0 || dbhdr == NULL) {
        return STATUS_ERROR;
    }

    // Allocate space for employees (plus one extra for potential add)
    size_t alloc_count = dbhdr->count + 1;
    struct employee_t *employees = calloc(alloc_count, sizeof(struct employee_t));
    if (employees == NULL) {
        perror("calloc");
        return STATUS_ERROR;
    }

    // Read employee records
    for (unsigned short i = 0; i < dbhdr->count; i++) {
        ssize_t bytes = read(fd, &employees[i], sizeof(struct employee_t));
        if (bytes != sizeof(struct employee_t)) {
            fprintf(stderr, "Failed to read employee %d\n", i);
            free(employees);
            return STATUS_ERROR;
        }
        // Convert hours from network byte order
        employees[i].hours = ntohl(employees[i].hours);
    }

    *employeesOut = employees;
    return STATUS_SUCCESS;
}

void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {
    if (dbhdr == NULL || employees == NULL) {
        printf("No employees to list\n");
        return;
    }

    printf("Database contains %d employee(s):\n", dbhdr->count);
    printf("%-30s %-40s %s\n", "Name", "Address", "Hours");
    printf("%-30s %-40s %s\n", "----", "-------", "-----");

    for (unsigned short i = 0; i < dbhdr->count; i++) {
        printf("%-30s %-40s %u\n",
               employees[i].name,
               employees[i].address,
               employees[i].hours);
    }
}

int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *addstring) {
    if (dbhdr == NULL || employees == NULL || addstring == NULL) {
        return STATUS_ERROR;
    }

    // Parse addstring format: "name,address,hours"
    char *str = strdup(addstring);
    if (str == NULL) {
        perror("strdup");
        return STATUS_ERROR;
    }

    struct employee_t *new_emp = &employees[dbhdr->count];
    memset(new_emp, 0, sizeof(struct employee_t));

    // First token: name
    char *token = strtok(str, ",");
    if (token == NULL) {
        fprintf(stderr, "Missing name\n");
        free(str);
        return STATUS_ERROR;
    }
    strncpy(new_emp->name, token, sizeof(new_emp->name) - 1);

    // Second token: address
    token = strtok(NULL, ",");
    if (token == NULL) {
        fprintf(stderr, "Missing address\n");
        free(str);
        return STATUS_ERROR;
    }
    strncpy(new_emp->address, token, sizeof(new_emp->address) - 1);

    // Third token: hours
    token = strtok(NULL, ",");
    if (token == NULL) {
        fprintf(stderr, "Missing hours\n");
        free(str);
        return STATUS_ERROR;
    }
    new_emp->hours = (unsigned int)atoi(token);

    free(str);

    dbhdr->count++;
    printf("Added employee: %s\n", new_emp->name);

    return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
    if (fd < 0 || dbhdr == NULL) {
        return STATUS_ERROR;
    }

    // Update filesize
    dbhdr->filesize = sizeof(struct dbheader_t) +
                      (dbhdr->count * sizeof(struct employee_t));

    // Seek to beginning
    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("lseek");
        return STATUS_ERROR;
    }

    // Write header in network byte order
    struct dbheader_t hdr_out;
    hdr_out.magic = htonl(dbhdr->magic);
    hdr_out.version = htons(dbhdr->version);
    hdr_out.count = htons(dbhdr->count);
    hdr_out.filesize = htonl(dbhdr->filesize);

    ssize_t bytes = write(fd, &hdr_out, sizeof(struct dbheader_t));
    if (bytes != sizeof(struct dbheader_t)) {
        fprintf(stderr, "Failed to write header\n");
        return STATUS_ERROR;
    }

    // Write employees (skip if NULL or count is 0)
    if (employees != NULL) {
        for (unsigned short i = 0; i < dbhdr->count; i++) {
            struct employee_t emp_out;
            memcpy(&emp_out, &employees[i], sizeof(struct employee_t));
            emp_out.hours = htonl(employees[i].hours);

            bytes = write(fd, &emp_out, sizeof(struct employee_t));
            if (bytes != sizeof(struct employee_t)) {
                fprintf(stderr, "Failed to write employee %d\n", i);
                return STATUS_ERROR;
            }
        }
    }

    return STATUS_SUCCESS;
}
