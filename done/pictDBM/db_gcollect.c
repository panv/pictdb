/**
 * @file db_gcollect.c
 * @brief Implements garbage collection.
 *
 * This feature is implemented using already defined functions.
 *
 * @author Vincenzo Bazzucchi
 * @author Nicolas Phan Van
 */

#include "pictDB.h"
#include "image_content.h" // For lazily resize


int do_gbcollect(struct pictdb_file* db_file, const char* db_name,
                 const char* tmp_name)
{
    // Check arguments
    if (db_file == NULL || db_name == NULL || tmp_name == NULL) {
        return ERR_INVALID_ARGUMENT;
    }
    if (strlen(db_name) == 0 || strlen(db_name) > MAX_DB_NAME) {
        return ERR_INVALID_FILENAME;
    }
    if (strlen(tmp_name) == 0 || strlen(tmp_name) > MAX_DB_NAME) {
        return ERR_INVALID_FILENAME;
    }

    // Initialize new file
    struct pictdb_file temp = {
        .fpdb = NULL, .header = db_file->header, .metadata = NULL
    };
    int ret = do_create(tmp_name, &temp);
    if (ret != 0) {
        return ret;
    }

    struct pict_metadata* pics = db_file->metadata; // Will be used often

    for (size_t i = 0; ret == 0 && i < db_file->header.max_files; ++i) {
        char* image = NULL;
        uint32_t size = 0;
        // Read image from old db and save it to the new one
        if (pics[i].is_valid == NON_EMPTY) {
            ret = do_read(pics[i].pict_id, RES_ORIG, &image, &size, db_file);
            ret = ret == 0 ? do_insert(image, size, pics[i].pict_id, &temp) : ret;
            // Resize the images that are resized in the old db.
            for (int r = 0; ret == 0 && r < RES_ORIG; ++r) {
                if (pics[i].size[r] != 0 && pics[i].offset[r] != 0) {
                    ret = lazily_resize(r, &temp, i);
                }
            }
            free(image);
        }
    }

    do_close(db_file); // Close old db before deleting it
    do_close(&temp);

    if (ret != 0) {
        remove(tmp_name);
    } else {
        // Remove old db and move new one
        ret = remove(db_name);
        ret = ret != -1 ? rename(tmp_name, db_name) : ret;
        ret = ret == -1 ? ERR_IO : ret;
    }

    return ret;
}
