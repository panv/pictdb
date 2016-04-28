/**
 * @file db_create.c
 * @brief pictDB library: do_create implementation.
 *
 * @author Mia Primorac
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

    // Sets the DB header name
    strncpy(db_file->header.db_name, filename, MAX_DB_NAME);
    db_file->header.db_name[MAX_DB_NAME] = '\0';
    // Initialize header
    db_file->header.db_version = 0;
    db_file->header.num_files = 0;

    // Open stream and check for errors
    FILE* output = fopen(filename, "wb");
    if (output == NULL) {
        fprintf(stderr, "Error : cannot open file %s\n", filename);
        return ERR_IO;
    }

    // Dynamically allocates memory to the metadata
    db_file->metadata = calloc(db_file->header.max_files,
                               sizeof(struct pict_metadata));
    // Check for allocation error
    if (db_file->metadata == NULL) {
        fclose(output);
        return ERR_OUT_OF_MEMORY;
    }

    // Sets all metadata validity to EMPTY (still useful after calloc?)
    for (uint32_t i = 0; i < db_file->header.max_files; ++i) {
        db_file->metadata[i].is_valid = EMPTY;
    }

    // Writes the header and the array of max_files metadata to file
    size_t header_ctrl = fwrite(&db_file->header,
                                sizeof(struct pictdb_header), 1, output);
    size_t metadata_ctrl = fwrite(db_file->metadata,
                                  sizeof(struct pict_metadata),
                                  db_file->header.max_files, output);
    // Free memory and overwrite pointer
    free(db_file->metadata);
    db_file->metadata = NULL;

    fclose(output);

    if (header_ctrl != 1 || metadata_ctrl != db_file->header.max_files) {
        fprintf(stderr, "Error : cannot create database %s\n",
                db_file->header.db_name);
        return ERR_IO;
    }

    printf("%zu item(s) written\n", header_ctrl + metadata_ctrl);
    return 0;
}
