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
    struct pict_metadata img_index = db_file->metadata[index];
    for (size_t i = 0; i < db_file->header.max_files; ++i) {
        if (i != index) { // For all images other than image at index
            if (db_file->metadata[i].is_valid == NON_EMPTY
                && !strncmp(db_file->metadata[i].pict_id, img_index.pict_id, MAX_PIC_ID)) {
                // Two distinct images have the same ID!
                return ERR_DUPLICATE_ID;
            } else if (!hashcmp(db_file->metadata[i].SHA, img_index.SHA)) {
                // Two images with the same hash: deduplication
                for (size_t res = 0; res < NB_RES; ++res) {
                    img_index.offset[res] = db_file->metadata[i].offset[res];
                    img_index.size[res] = db_file->metadata[i].size[res];
                }
                return 0;
            }
        }
    }
    // No duplicates
    img_index.offset[RES_ORIG] = 0;
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
