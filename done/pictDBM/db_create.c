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

int do_create(const char filename[], struct pictdb_file* db_file) {
    // Sets the DB header name
    strncpy(db_file->header.db_name, CAT_TXT,  MAX_DB_NAME);
    db_file->header.db_name[MAX_DB_NAME] = '\0';

    // Initialize header
    db_file->header.db_version = 0;
    db_file->header.num_files = 0;
    db_file->header.max_files = MAX_MAX_FILES;

    // Open stream and check for errors
    FILE* output = fopen(filename, "wb");
    if (output == NULL) {
        fprintf(stderr,
                "Impossible ouvrir le fichier %s en écriture\n", filename);
        return ERR_INVALID_FILENAME;
    }

    // No need to initialize .metadata
    size_t total = 0; // Used to store the total number of object written by fwrite

    // Write header and check for error
    size_t written_els = fwrite(&db_file->header, sizeof(db_file->header), 1, output);
    if (written_els != 1) {
        fprintf(stderr, "Impossible créer la base de données %s\n", db_file->header.db_name);
        return ERR_IO;
    }
    ++total; // Header counts as one element

    // Write metadata and check for error
    written_els = fwrite(&db_file->metadata, sizeof(db_file->metadata), 1, output);
    if (written_els != 1) {
        fprintf(stderr, "Impossible créer la base de données %s\n", db_file->header.db_name);
        return ERR_IO;
    }
    total += db_file->header.num_files; // ??? Correct

    fclose(output);
    printf("%zu item(s) written\n", total);
    return 0;
}
