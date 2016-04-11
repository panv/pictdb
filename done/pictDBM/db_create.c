/**
 * @file db_create.c
 * @brief pictDB library: do_create implementation.
 *
 * @author Mia Primorac
 * @date 2 Nov 2015
 */

#include "pictDB.h"

#include <string.h> // for strncpy and strlen

/*
 * @brief Returns an empty pict_metadata object
 *
 * @return An empty pict_metadata object (is_valid = EMTPY)
 */
struct pict_metadata empty_metadata(void);

/********************************************************************//**
 * Creates the database called db_filename. Writes the header and the
 * preallocated empty metadata array to database file.
 */

int do_create(const char* filename, struct pictdb_file* db_file)
{
    // Error checks
    if (filename == NULL || strlen(filename) > MAX_DB_NAME) {
        return ERR_INVALID_FILENAME;
    }
    if (db_file->header.max_files > MAX_MAX_FILES) {
        return ERR_MAX_FILES;
    }

    // Sets the DB header name
    strncpy(db_file->header.db_name, filename, MAX_DB_NAME);
    db_file->header.db_name[MAX_DB_NAME] = '\0';

    // Initialize header
    db_file->header.db_version = 0;
    db_file->header.num_files = 0;

    // Sets all metadata validity to EMPTY
    for (uint32_t i = 0; i < db_file->header.max_files; ++i) {
        db_file->metadata[i] = empty_metadata();
    }

    // Open stream and check for errors
    FILE* output = fopen(filename, "wb");
    if (output == NULL) {
        fprintf(stderr,
                "Error : cannot open file %s\n", filename);
        return ERR_IO;
    }

    // Writes the header and the array of max_files metadata to file
    size_t header_ctrl = fwrite(&db_file->header,
                                sizeof(struct pictdb_header), 1, output);

    size_t metadata_ctrl = fwrite(&db_file->metadata,
                                  sizeof(struct pict_metadata),
                                  db_file->header.max_files, output);

    if (header_ctrl != 1 || metadata_ctrl != db_file->header.max_files) {
        fprintf(stderr, "Error : cannot create database %s\n",
                db_file->header.db_name);
        return ERR_IO;
    }

    fclose(output);
    printf("%zu item(s) written\n", header_ctrl + metadata_ctrl);
    return 0;
}

/**
 * @brief Returns an empty metadata.
 */
struct pict_metadata empty_metadata(void)
{
    struct pict_metadata metadata = {.is_valid = EMPTY};
    return metadata;
}

