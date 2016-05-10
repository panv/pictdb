/**
 * @file dedup.c
 * @brief Implements image deduplication feature.
 *
 * @author Vincenzo Bazzucchi
 * @author Nicolas Phan Van
 */

#include "dedup.h"

/**
 * @brief Compares two hashes digests.
 *
 * @param h1, h2 The two hashes to compare.
 * @return 0 if h1 equals h2, 1 otherwise.
 */
int hashcmp(unsigned char* h1, unsigned char* h2);

int do_name_and_content_dedup(struct pictdb_file* db_file, uint32_t index)
{
    if (db_file == NULL || db_file->metadata[index].is_valid == EMPTY) {
        return ERR_INVALID_ARGUMENT;
    }

    struct pict_metadata* img_index = &db_file->metadata[index];
    int found = 0;
    for (size_t i = 0; i < db_file->header.max_files; ++i) {
        // For all valid images other than the one at index
        if (i != index && db_file->metadata[i].is_valid == NON_EMPTY) {
            // Two distinct images have the same ID!
            if (strncmp(db_file->metadata[i].pict_id,
                        img_index->pict_id, MAX_PIC_ID) == 0) {
                return ERR_DUPLICATE_ID;
            } else if (found == 0 &&
                       hashcmp(db_file->metadata[i].SHA, img_index->SHA) == 0) {
                // Two images with the same hash: deduplication
                for (size_t res = 0; res < NB_RES; ++res) {
                    img_index->offset[res] = db_file->metadata[i].offset[res];
                    img_index->size[res] = db_file->metadata[i].size[res];
                }
                found = 1;
            }
        }
    }

    // No duplicates found
    if (found == 0) {
        img_index->offset[RES_ORIG] = 0;
    }

    return 0;
}

int hashcmp(unsigned char* h1, unsigned char* h2)
{
    for (size_t i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        if (h1[i] != h2[i]) {
            return 1;
        }
    }
    return 0;
}
