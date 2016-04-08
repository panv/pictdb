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

/**
 * @brief Finds the index of an image in an array of metadata
 *
 * @param pict_id The identifier of the image to be found.
 * @param images The array of metadata.
 * @param db_size The length of the array.
 * @param index Pointer to the variable where to write the index of the image.
 *
 * @return 0 if no error occurs, an error coded in error.h in case of error
 */
int index_of_image(const char* pict_id, const struct pict_metadata images[],
        const uint32_t db_size, uint32_t* index);

/*
 * @brief Checks for equality between two strings
 *
 * @param s1, s2 The two strings to compare.
 *
 * @return A non-zero value if the strings are equal, else 0.
 */
int equal_string(const char* s1, const char* s2);


int do_delete(struct pictdb_file* db_file, const char* pict_id)
{
    if (db_file == NULL || pict_id == NULL){
        return ERR_INVALID_ARGUMENT;
    }

    // Find index of image to remove
    uint32_t index;
    int found = index_of_image(pict_id, db_file->metadata,
            db_file->header.num_files, &index);
    if (found == ERR_FILE_NOT_FOUND) {
        return ERR_FILE_NOT_FOUND;
    }

    // Mark the image as invalid
    db_file->metadata[index].is_valid = EMPTY;

    // Update header
    ++db_file->header.db_version;
    --db_file->header.num_files;

    /* TODO: Write dans file using fseek and fwrite */
}


int index_of_image(const char* pict_id, const struct pict_metadata images[],
        const uint32_t db_size, uint32_t* index)
{
    for (uint32_t i = 0; i < db_size; ++i) {
        if (equal_string(pict_id, images[i].pict_id)){
            *index = i;
            return 0;
        }
    }
    return ERR_FILE_NOT_FOUND;
}

int equal_string(const char* s1, const char* s2)
{
    int are_equal = 1;
    for (size_t i = 0; i < strlen(s1); ++i){
        are_equal = (s1[i] == s2[i]) && are_equal;
    }
    return are_equal && (strlen(s1) == strlen(s2));
}

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
    db_file->fpdb = input_stream;

    size_t read_els = fread(&db_file->header, sizeof(struct pictdb_header), 1,
            input_stream);
    if (read_els != 1) {
        fprintf(stderr, "Could not read header from %s\n", filename);
        return ERR_IO;
    }

    read_els = fread(&db_file->metadata, sizeof(struct pict_metadata),
            db_file->header.num_files, input_stream);
    if(read_els != db_file->header.num_files) {
        fprintf(stderr, "Could not read metadata from %s\n", filename);
        return ERR_IO;
    }

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
void print_header(const struct pictdb_header* header) {
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
void print_metadata (const struct pict_metadata* metadata) {
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
