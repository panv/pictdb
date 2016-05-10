/**
 * @file db_create.c
 * @brief pictDB library: do_create implementation.
 *
 * @date 2 Nov 2015
 */

#include "pictDB.h"

int do_create(const char* filename, struct pictdb_file* db_file)
{
    // Error checks
    if (filename == NULL || db_file == NULL) {
        return ERR_INVALID_ARGUMENT;
    }
    if (strlen(filename) == 0 || strlen(filename) > MAX_DB_NAME) {
        return ERR_INVALID_FILENAME;
    }

    // Open stream and check for errors
    db_file->fpdb = fopen(filename, "wb");
    if (db_file->fpdb == NULL) {
        fprintf(stderr, "Error : cannot open file %s\n", filename);
        return ERR_IO;
    }

    // Sets the DB header name
    strncpy(db_file->header.db_name, filename, MAX_DB_NAME);
    db_file->header.db_name[MAX_DB_NAME] = '\0';
    // Initialize header
    db_file->header.db_version = 0;
    db_file->header.num_files = 0;

    // Dynamically allocates memory to the metadata
    db_file->metadata = calloc(db_file->header.max_files,
                               sizeof(struct pict_metadata));
    // Check for allocation error
    if (db_file->metadata == NULL) {
        return ERR_OUT_OF_MEMORY;
    }

    // Writes the header and the array of max_files metadata to file
    size_t header_ctrl = fwrite(&db_file->header,
                                sizeof(struct pictdb_header), 1, db_file->fpdb);
    size_t metadata_ctrl = fwrite(db_file->metadata,
                                  sizeof(struct pict_metadata),
                                  db_file->header.max_files, db_file->fpdb);
    if (header_ctrl != 1 || metadata_ctrl != db_file->header.max_files) {
        fprintf(stderr, "Error : cannot create database %s\n",
                db_file->header.db_name);
        //correcteur: libérez la mémoire en cas d'erreur
        return ERR_IO;
    }

    printf("%zu item(s) written\n", header_ctrl + metadata_ctrl);
    return 0;
}
