#include "dedup.h"
#include<stdlib.h>
#include <openssl/sha.h>

int do_name_and_content_dedup(struct pictdb_file* db_file, uint32_t index)
{
    struct pict_metadata img_index = db_file->metadata[index];
    for (size_t i = 0; i < db_file->header.max_files; ++i) {
        if (i != index) {
            if (db_file->metadata[i].is_valid == NON_EMPTY &&
                    !strcmp(db_file->metadata[i].pict_id, img_index.pict_id)) {
                return ERR_DUPLICATE_ID;
            }
            if (!strncmp(db_file->metadata[i].SHA, img_index.SHA, SHA256_DIGEST_LENGTH)) {
                img_index.offsets[RES_ORIG] = 0;
                return 0;
            }
            else {
                for (size_t i = 0; i < NB_RES; ++i) {
                    img_index.offset[i] = db_file->metadata.offsets[i];
                    img_index.size[i] = db_file->metadata.size[i];
                }
                return 0;
            }
        }
    }
}

unsigned char* hash_of_image(struct pictdb_file* db_file, uint32_t index)
{
    // destination preparation
    unsigned char* hash = calloc(SHA256_DIGEST_LENGTH, sizeof(char));

    // Initialization of sha reaction
    SHA256_CTX hash_calculator;
    SHA256_Init(&hash_calculator);

    // input preparation
    uint32_t img_size = db_file->metadata[index].size[RES_ORIG];
    fseek(db_file->fpdb, db_file->metadata[index].offset[RES_ORIG], SEEK_SET);
    char image[img_size];
    fread(image, img_size, 1, db_file->fpdb);

    // Run over data
    SHA256_Update(&hash_calculator, image, img_size);

    //Extract hash
    SHA256_Final(hash, &hash_calculator);
    return hash;
}
