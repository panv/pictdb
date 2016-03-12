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
void print_header(const struct pictdb_header header) {
	printf("*****************************************\n
			**********DATABASE HEADER START**********\n
			DB NAME: %31s \n
			VERSION: %" PRIu32 " \n
			IMAGE COUNT: %" PRIu32 " \t\t MAX IMAGES: %" PRIu32 "\n
			THUMBNAIL: %" PRIu16 " x %" PRIu16" \t SMALL: %" PRIu16 " x %"PRIu16" \n
			***********DATABASE HEADER END*********** \n
			"***************************************** ",
			header.db_name, header.db_version, header.num_files,
			header.max_files, header.res_resized[0], header.res_resized[1],
			header.res_resized[2], header.res_resized[3]);
}

/********************************************************************//**
 * Metadata display.
 */
void print_metadata (const struct pict_metadata metadata)
{
    char sha_printable[2*SHA256_DIGEST_LENGTH+1];
    sha_to_string(metadata.SHA, sha_printable);

    /* **********************************************************************
     * TODO: WRITE YOUR CODE HERE
     * **********************************************************************
     */
}
