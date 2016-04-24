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

    // Open stream and check for errors
    FILE* output = fopen(filename, "wb");
    if (output == NULL) {
        fprintf(stderr, "Error : cannot open file %s\n", filename);
        return ERR_IO;
    }

    // Sets the DB header name
    strncpy(db_file->header.db_name, filename, MAX_DB_NAME);
    db_file->header.db_name[MAX_DB_NAME] = '\0';
    // Initialize header
    db_file->header.db_version = 0;
    db_file->header.num_files = 0;

    // Writes the header and the array of max_files metadata to file
    if (fwrite(&db_file->header, sizeof(struct pictdb_header), 1, output) != 1) {
        fprintf(stderr, "Error : cannot write header of database %s\n",
                db_file->header.db_name);
        fclose(output);
        return ERR_IO;
    }

    uint32_t written_elem = 1;
    for (uint32_t i = 0; i < db_file->header.max_files; ++i) {
        // Dynamically allocates memory to the metadata
        struct pict_metadata* metadata = malloc(sizeof(struct pict_metadata));
        // Check for allocation error
        if (metadata == NULL) {
            fclose(output);
            return ERR_OUT_OF_MEMORY;
        }
        metadata->is_valid = EMPTY;
        if (fwrite(metadata, sizeof(struct pict_metadata), 1, output) != 1) {
            fprintf(stderr, "Error : cannot write metadata of database %s\n",
                    db_file->header.db_name);
            free(metadata);
            fclose(output);
            return ERR_IO;
        }
        free(metadata);
        ++written_elem;
    }

    fclose(output);

    printf("%" PRIu32 " item(s) written\n", written_elem);
    return 0;
}
