/**
 * @file db_delete.c
 * @brief pictDB library: do_delete implementation.
 *
 * @author Vincenzo Bazzucchi and Nicolas Phan Van
 * @date 9 Apr 2016
 */

#include "pictDB.h"
#include <string.h>
/**
 * @brief Finds the index of an image in an array of metadata
 *
 * @param pict_id The identifier of the image to be found.
 * @param images The array of metadata.
 * @param db_size The length of the array.
 * @param index Pointer to the variable where to write the index of the image.
 *
 * @return 0 if no error occurs, an error coded in error.h in case of error
 */
int index_of_image(const char* pict_id, const struct pict_metadata images[],
                   const uint32_t db_size, uint32_t* index);

int do_delete(struct pictdb_file* db_file, const char* pict_id)
{
    if (db_file == NULL || pict_id == NULL) {
        return ERR_INVALID_ARGUMENT;
    }
    if (strlen(pict_id) > MAX_PIC_ID) {
        return ERR_INVALID_PICID;
    }

    // Find index of image to remove
    uint32_t index;
    int found = index_of_image(pict_id, db_file->metadata,
                               db_file->header.max_files, &index);
    if (found != 0) {
        return ERR_FILE_NOT_FOUND;
    }

    /*
    // No need to delete an image that is not valid
    if (db_file->metadata[index].is_valid == EMPTY) {
        return 0;
    }
    */
    // Mark the image as invalid
    db_file->metadata[index].is_valid = EMPTY;

    // Position write head after the header
    int seek_success = fseek(db_file->fpdb, sizeof(struct pictdb_header), SEEK_SET);
    if (seek_success == 0) {
        // Write metadata
        size_t write_success = fwrite(db_file->metadata, sizeof(struct pict_metadata),
                                      db_file->header.num_files, db_file->fpdb);
        if (write_success == db_file->header.num_files) {
            // Update header
            ++db_file->header.db_version;
            --db_file->header.num_files;
            // Position write head at beginning of file
            seek_success = fseek(db_file->fpdb, 0, SEEK_SET);
            if (seek_success == 0) {
                // Write header
                write_success = fwrite(&db_file->header, sizeof(struct pictdb_header), 1, db_file->fpdb);
                return write_success == 1 ? 0 : ERR_IO;
            }
        }
    }
    return ERR_IO;
}

int index_of_image(const char* pict_id, const struct pict_metadata images[],
                   const uint32_t db_size, uint32_t* index)
{
    for (uint32_t i = 0; i < db_size; ++i) {
        if (strcmp(pict_id, images[i].pict_id) == 0
            && images[i].is_valid == NON_EMPTY) {
            *index = i;
            return 0;
        }
    }
    return ERR_FILE_NOT_FOUND;
}
