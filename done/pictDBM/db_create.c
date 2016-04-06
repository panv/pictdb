/**
 * @file db_create.c
 * @brief pictDB library: do_create implementation.
 *
 * @author Mia Primorac
 * @date 2 Nov 2015
 */

#include "pictDB.h"

#include <string.h> // for strncpy

/********************************************************************//**
 * Creates the database called db_filename. Writes the header and the
 * preallocated empty metadata array to database file.
 */

int do_create(const char filename[], struct pictdb_file db_file) {
    // Sets the DB header name
    strncpy(db_file.header.db_name, CAT_TXT,  MAX_DB_NAME);
    db_file.header.db_name[MAX_DB_NAME] = '\0';

    // Initialize header
    db_file.header.db_version = 0;
    db_file.header.num_files = 0;
    db_file.header.max_files = MAX_MAX_FILES; // already done in do_create_cmd, remove?

    FILE* output = fopen(filename, "wb");
    if (output == NULL) {
        fprintf(stderr,
                "Error : cannot open file %s\n", filename);
        return ERR_INVALID_FILENAME;
    }
    
    db_file.fpdb = output; // maybe, i don't know
    
    for (size_t i = 0; i < db_file.header.max_files; ++i) {
        db_file.metadata[i] = empty_metadata;
    }

    size_t header_ctrl = fwrite(&db_file.header, sizeof(db_file.header), 1, output);
    size_t metadata_ctrl = fwrite(&db_file.metadata, sizeof(db_file.metadata), 1, output);
    if (header_ctrl != 1 || metadata_ctrl != 1) {
        fprintf(stderr, "Error : cannot create database %s\n", db_file.header.db_name);
        return ERR_IO;
    }
    size_t total = 1 + db_file.header.num_files;

    fclose(output);
    printf("%zu item(s) written\n", total);
    return 0;
}

struct pict_metadata empty_metadata(void) {
    struct pict_metadata metadata;
    metadata.is_valid = EMPTY;
    return metadata;
}
