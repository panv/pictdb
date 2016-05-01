#include "pictDB.h"
#include "image_content.h"

int do_read(const char* pict_id, int resolution, char** image_address,
        uint32_t* size, struct pictdb_file* db_file)
{
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
    char* image_buffer = calloc(to_read->size[resolution], sizeof(char)); //CC
    if (image_buffer == NULL) {
        return ERR_OUT_OF_MEMORY;
    }
    if (fseek(db_file->fpdb, to_read->offset[resolution], SEEK_SET) ||
            fread(image_buffer, to_read->size[resolution], sizeof(char), db_file->fpdb) != to_read->size[resolution]) {
        free(image_buffer);
        return ERR_IO;
    }
    *size = to_read->size[resolution];
    *image_address = image_buffer;
    return 0;
}

