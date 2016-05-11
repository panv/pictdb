#include "pictDB.h"
#include "dedup.h"
#include "image_content.h"

#define RET_ERROR if (ret != 0) return ret
// fseek + error check
#define SEEK(offset, whence) \
    ret = fseek(db_file->fpdb, offset, whence) == 0 ? 0 : ERR_IO
// fwrite + error check
#define WRITE(src, size) \
    ret = fwrite(src, size, 1, db_file->fpdb) == 1 ? 0 : ERR_IO


int do_insert(const char* new_image, size_t size, const char* pict_id,
              struct pictdb_file* db_file)
{
    // Argument check
    if (new_image == NULL || pict_id == NULL || db_file == NULL
        || size == 0) {
        return ERR_INVALID_ARGUMENT;
    }
    if (strlen(pict_id) == 0 || strlen(pict_id) > MAX_PIC_ID) {
        return ERR_INVALID_PICID;
    }
    if (!(db_file->header.num_files < db_file->header.max_files)) {
        return ERR_FULL_DATABASE;
    }

    // Find index of first empty metadata
    uint32_t idx_new = 0;
    while (db_file->metadata[idx_new].is_valid == NON_EMPTY) {
        ++idx_new;
    }

    struct pict_metadata* empty = &db_file->metadata[idx_new]; // convenience

    // Update metadata with image information
    (void)SHA256((unsigned char*)new_image, size, empty->SHA);  //add checksum
    strncpy(empty->pict_id, pict_id, MAX_PIC_ID + 1); //+1 necessary?
    empty->size[RES_ORIG] = (uint32_t) size; //Pourquoi attention au changement de type?
    empty->is_valid = NON_EMPTY;

    // Deduplication
    int ret = do_name_and_content_dedup(db_file, idx_new);
    RET_ERROR;
    // Image does not already exist in the database, write it at the end
    if (empty->offset[RES_ORIG] == 0) {
        SEEK(0, SEEK_END);
        if (ret == 0) {
            empty->offset[RES_ORIG] = ftell(db_file->fpdb);
            WRITE(new_image, size);
        }
        RET_ERROR;
    }

    // Update metadata with image resolution
    ret = get_resolution(&empty->res_orig[1], &empty->res_orig[0],
                         new_image, size);
    RET_ERROR;

    // Update and write header
    ++db_file->header.db_version;
    ++db_file->header.num_files;
    SEEK(0, SEEK_SET);
    if (ret == 0) {
        WRITE(&db_file->header, sizeof(struct pictdb_header));
    }

    // Write metadata
    if (ret == 0) {
        uint64_t meta_offset = sizeof(struct pictdb_header) +
                               idx_new * sizeof(struct pict_metadata);
        SEEK(meta_offset, SEEK_SET);
        if (ret == 0) {
            WRITE(empty, sizeof(struct pict_metadata));
        }
    }

    return ret;
}
