#include "pictDB.h"
#include "dedup.h"
#include "image_content.h"

int do_insert(const char* new_image, size_t size, const char* pict_id,
              struct pictdb_file* db_file)
{
    if (!(db_file->header.num_files < db_file->header.max_files)) {
        return ERR_FULL_DATABASE;
    }

    uint32_t idx_new = 0; //find index of first empty metadata
    while (db_file->metadata[idx_new].is_valid != EMPTY) {
        ++idx_new;
    }
    struct pict_metadata* empty = &db_file->metadata[idx_new]; // convenience

    (void)SHA256((unsigned char*)new_image, size, empty->SHA);  //add checksum
    strncpy(empty->pict_id, pict_id, MAX_PIC_ID + 1); //+1 necessary?
    empty->size[RES_ORIG] = (uint32_t) size; //Pourquoi attention au changement de type?
    empty->is_valid = NON_EMPTY;

    int dedup_err = do_name_and_content_dedup(db_file, idx_new); //dedup
    if (dedup_err) {
        return dedup_err;
    } else if (empty->offset[RES_ORIG] == 0) {
        fseek(db_file->fpdb, 0, SEEK_END); //CC
        empty->offset[RES_ORIG] = ftell(db_file->fpdb); //CC
        fwrite(new_image, size, 1, db_file->fpdb); //CC
    }
    int found_res = get_resolution(&empty->res_orig[0], &empty->res_orig[1],
                                   new_image, size); //CC
    ++db_file->header.db_version;
    ++db_file->header.num_files;
    fseek(db_file->fpdb, 0, SEEK_SET);
    fwrite(&db_file->header, sizeof(struct pictdb_header), 1, db_file->fpdb); //C

    // PROBLEM: find where to write metadata. Is this correct?
    uint64_t meta_offset = sizeof(struct pictdb_header) +
                           idx_new * sizeof(struct pict_metadata);
    fseek(db_file->fpdb, meta_offset, SEEK_SET); //CC
    fwrite(empty, sizeof(struct pict_metadata), 1, db_file->fpdb); //C
    return 0;
}
