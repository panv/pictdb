#include "pictDB.h"
#include "image_content.h"

#define ASSIGN_NEW_VALUE(oldvalue, newvalue) \
    oldvalue = newvalue != 0 ? newvalue : oldvalue

int content_dedup(struct pictdb_file* db_file, uint32_t index);


int do_read(const char* pict_id, int resolution, char** image_buffer,
            uint32_t* image_size, struct pictdb_file* db_file)
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
    int ret = ERR_FILE_NOT_FOUND;
    for (uint32_t i = 0; i < db_file->header.max_files
         && ret == ERR_FILE_NOT_FOUND; ++i) {
        if (db_file->metadata[i].is_valid == NON_EMPTY &&
            strncmp(pict_id, db_file->metadata[i].pict_id, MAX_PIC_ID) == 0) {
            idx = i;
            ret = 0;
        }
    }
    if (ret != 0) {
        return ERR_FILE_NOT_FOUND;
    }

    struct pict_metadata* to_read = &db_file->metadata[idx];
    if (content_dedup(db_file, idx) != 0) {
        if (fseek(db_file->fpdb, sizeof(struct pictdb_header) +
                  idx * sizeof(struct pict_metadata), SEEK_SET) ||
            fwrite(to_read, sizeof(struct pict_metadata), 1, db_file->fpdb) != 1) {
            return ERR_IO;
        }
    }

    if (resolution != RES_ORIG && (to_read->offset[resolution] == 0 ||
                                   to_read->size[resolution] == 0)) {
        ret = lazily_resize(resolution, db_file, idx);
        if (ret != 0) {
            return ret;
        }
    }
    *image_buffer = malloc(to_read->size[resolution]);
    if (*image_buffer == NULL) {
        return ERR_OUT_OF_MEMORY;
    }
    if (fseek(db_file->fpdb, to_read->offset[resolution], SEEK_SET) ||
        fread(*image_buffer, to_read->size[resolution], 1,
              db_file->fpdb) != 1) {
        free(*image_buffer);
        return ERR_IO;
    }
    *image_size = to_read->size[resolution];
    return 0;
}

int content_dedup(struct pictdb_file* db_file, uint32_t index)
{
    struct pict_metadata* img_index = &db_file->metadata[index];
    int found = 0;
    for (size_t i = 0; i < db_file->header.max_files; ++i) {
        if (i != index && db_file->metadata[i].is_valid == NON_EMPTY) {
            if (hashcmp(db_file->metadata[i].SHA, img_index->SHA) == 0) {
                for (size_t res = 0; res < NB_RES; ++res) {
                    ASSIGN_NEW_VALUE(img_index->offset[res],
                                     db_file->metadata[i].offset[res]);
                    ASSIGN_NEW_VALUE(img_index->size[res],
                                     db_file->metadata[i].size[res]);
                    found = found || (img_index->size[res] != 0);
                }
            }
        }
    }
    return found;
}
