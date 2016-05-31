/**
 * @file db_gcollect.c
 * @brief Implements garbage collection.
 * This feature is implemented using already defined functions.
 *
 * @author Vincenzo Bazzucchi
 * @author Nicolas Phan Van
 */

#include "pictDB.h"
#include "image_content.h" // For lazily resize.
#define RET_ERROR if (ret != 0) return ret

/**
 * @brief Cleans the database file by eliminating the holes created when deleting pictures.
 * This is done by creating a new db and reinserting all the images.
 *
 * @param db_file The database file to be cleaned.
 * @param dbname The filename of the database to be cleaned.
 * @param tmp_name The filename of the temporary database.
 *
 * @return 0 if no error occurred, an int coded in error.h in case of error.
 */
int do_gbcollect(struct pictdb_file* db_file, const char* db_name,
             const char* tmp_name)
{
    // Check arguments.
    if (db_file == NULL || db_name == NULL || tmp_name == NULL) {
        return ERR_INVALID_ARGUMENT;
    }

    // Should I consider that each GC increments the db_version by 1?
    uint32_t old_version =
        db_file->header.db_version; //Save old version + 1 after GC, before write

    FILE* f_temp = fopen(tmp_name, "wb"); // Open tmp file and check pointer.
    if (f_temp == NULL) {
        fprintf(stderr, "Impossible to open %s\n", tmp_name);
        return ERR_IO;
    }

    // Create new header
    struct pictdb_header h_temp = {
        .max_files = db_file->header.max_files,
        .unused_32 = db_file->header.unused_32,
        .unused_64 = db_file->header.unused_64
    };
    strcpy(h_temp.db_name, db_file->header.db_name);
    // REMPLACER LIGNE SUIVANTE PAR FOR LOOP
    memcpy(h_temp.res_resized, db_file->header.res_resized,
           (2 * (NB_RES  - 1)) * sizeof(uint16_t));

    // Initialize new file.
    struct pictdb_file temp = {f_temp, h_temp, NULL};
    int ret = do_create(db_name, &temp);
    RET_ERROR;


    struct pict_metadata* pics = db_file->metadata; // Will be used often.

    for (uint32_t i = 0; i < db_file->header.num_files; ++i) {
        char* image = NULL;
        uint32_t size = 0;
        // Read image from old db and save it to the new one.
        ret = do_read(pics[i].pict_id, RES_ORIG, &image, &size, db_file);
        RET_ERROR;
        ret = do_insert(image, size, pics[i].pict_id, &temp);
        RET_ERROR;
        // Resize the images that are resized in the old db.
        for (int r = 0; r < RES_ORIG; ++r) {
            if (pics[i].size[r] == 0) {
                ret = lazily_resize(r, &temp, i);
                RET_ERROR;
            }
        }
    }
    temp.header.db_version = old_version + 1; // Assign correct version of db.

    // Remove old db and move new one.
    do_close(db_file); //Close old db before deleting it.
    ret = remove(db_name);
    RET_ERROR;
    do_close(&temp); // Should I close? If yes, should it be opened again?
    ret = rename(tmp_name, db_name);
    RET_ERROR;

    return 0;
}
