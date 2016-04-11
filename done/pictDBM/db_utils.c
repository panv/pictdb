/* ** NOTE: undocumented in Doxygen
 * @file db_utils.c
 * @brief implementation of several tool functions for pictDB
 *
 * @author Mia Primorac
 * @date 2 Nov 2015
 */

#include "pictDB.h"
#include <stdint.h> // for uint8_t
#include <stdio.h> // for sprintf
#include <openssl/sha.h> // for SHA256_DIGEST_LENGTH
#include <string.h> // for strlen
#include <inttypes.h>

int do_open(const char* filename, const char* mode,
            struct pictdb_file* db_file)
{
    if (filename == NULL || mode == NULL || db_file == NULL) {
        fprintf(stderr,
                "Invalid pointer\n");
        return ERR_INVALID_ARGUMENT;
    }

    FILE* input_stream = fopen(filename, mode);
    if (input_stream == NULL) {
        fprintf(stderr, "Error opening %s\n", filename);
        return ERR_IO;
    }

    size_t read_els = fread(&db_file->header, sizeof(struct pictdb_header), 1,
                            input_stream);
    if (read_els != 1) {
        fprintf(stderr, "Could not read header from %s\n", filename);
        return ERR_IO;
    }

    read_els = fread(&db_file->metadata, sizeof(struct pict_metadata),
                     db_file->header.max_files, input_stream);
    if(read_els != db_file->header.max_files) {
        fprintf(stderr, "Could not read metadata from %s\n", filename);
        return ERR_IO;
    }

    db_file->fpdb = input_stream; // shouldn't it be affectected after a successful read?
    return 0;
}

void do_close(struct pictdb_file* db_file)
{
    if (db_file != NULL && db_file->fpdb != NULL) {
        fclose(db_file->fpdb);
    }
}

/********************************************************************//**
 * Human-readable SHA
 */
static void
sha_to_string (const unsigned char* SHA,
               char* sha_string)
{
    if (SHA == NULL) {
        return;
    }
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        sprintf(&sha_string[i*2], "%02x", SHA[i]);
    }

    sha_string[2*SHA256_DIGEST_LENGTH] = '\0';
}

/********************************************************************//**
 * pictDB header display.
 */

void print_header(const struct pictdb_header* header)
{
    printf("*****************************************\n"
           "**********DATABASE HEADER START**********\n"
           "DB NAME: %31s\n"
           "VERSION: %" PRIu32 "\n"
           "IMAGE COUNT: %" PRIu32 "\t\tMAX IMAGES: %" PRIu32 "\n"
           "THUMBNAIL: %" PRIu16 " x %" PRIu16 "\t"
           "SMALL: %" PRIu16 " x %" PRIu16 "\n"
           "***********DATABASE HEADER END***********\n"
           "*****************************************\n",
           header->db_name, header->db_version, header->num_files,
           header->max_files, header->res_resized[0], header->res_resized[1],
           header->res_resized[2], header->res_resized[3]);
}

/********************************************************************//**
 * Metadata display.
 */

/*
 *
 * @brief Prints the content of a metadata object
 *
 * @param metadata In memory object representing the metadata
 * describing an image.
 */
void print_metadata (const struct pict_metadata* metadata)
{
    char sha_printable[2 * SHA256_DIGEST_LENGTH + 1];
    sha_to_string(metadata->SHA, sha_printable);
    printf(
        "PICTURE ID: %s\n"
        "SHA: %s\n"
        "VALID: %" PRIu16 "\n"
        "UNUSED: %" PRIu16 "\n"
        "OFFSET ORIG. : %" PRIu64 "\t\tSIZE ORIG. : %" PRIu32 "\n"
        "OFFSET THUMB.: %" PRIu64 "\t\tSIZE THUMB.: %" PRIu32 "\n"
        "OFFSET SMALL : %" PRIu64 "\t\tSIZE SMALL : %" PRIu32 "\n"
        "ORIGINAL: %" PRIu32 " x %" PRIu32 "\n"
        "*****************************************\n",
        metadata->pict_id, sha_printable, metadata->is_valid,
        metadata->unused_16,
        metadata->offset[RES_ORIG], metadata->size[RES_ORIG],
        metadata->offset[RES_THUMB], metadata->size[RES_THUMB],
        metadata->offset[RES_SMALL], metadata->size[RES_SMALL],
        metadata->res_orig[0], metadata->res_orig[1]);
}
