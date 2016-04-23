#include "dedup.h"

int do_name_and_content_dedup(struct pictdb_file* db_file, uint32_t index) {
    for (size_t i; i < db_file->header.max_files; ++i) {
        if (i != index && db_file->metadata[i].is_valid == NON_EMPTY) {
            if (strcmp(db_file->metadata[i].pict_id,
                       db_file->metadata[index].pict_id) == 0) {
                return ERR_DUPLICATE_ID;
            }
            if (qqc) {
                faire_des_trucs();
            }
        }
    }
}
