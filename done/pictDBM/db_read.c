#include "pictDB.h"
#include "image_content.h"

int do_read(const char* pict_id, int resolution, char** image,
            uint32_t* size, struct pictdb_file* db_file)
{
    if (pict_id == NULL || db_file == NULL) {
        return ERR_INVALID_ARGUMENT;
    }
    if (strlen(pict_id) == 0 || strlen(pict_id) > MAX_PIC_ID) {
        return ERR_INVALID_PICID;
    }
    if (db_file->header.num_files == 0) {
        return ERR_FILE_NOT_FOUND;
    }

    uint32_t idx = 0;
    for (uint32_t i = 0; i < db_file->header.num_files; ++i) {
        if (db_file->metadata[i].is_valid == NON_EMPTY &&
            !strncmp(pict_id, db_file->metadata[i].pict_id, MAX_PIC_ID)) {
            idx = i;
        }
    }
    if (strncmp(pict_id, db_file->metadata[idx].pict_id, MAX_PIC_ID)) {
        return ERR_FILE_NOT_FOUND;
    }
    struct pict_metadata* to_read = &db_file->metadata[idx];
    if (resolution != RES_ORIG && (to_read->offset[resolution] == 0 ||
                                   to_read->size[resolution] == 0)) {
        int resized = lazily_resize(resolution, db_file, idx);
        if (resized != 0) {
            return resized;
        }
    }
    *image = calloc(to_read->size[resolution], sizeof(char)); //CC
    if (*image == NULL) {
        return ERR_OUT_OF_MEMORY;
    }
    if (fseek(db_file->fpdb, to_read->offset[resolution], SEEK_SET) ||
        fread(*image, to_read->size[resolution], sizeof(char),
              db_file->fpdb) != to_read->size[resolution]) {
        free(*image);
        return ERR_IO;
    }
    *size = to_read->size[resolution];
    return 0;
}

