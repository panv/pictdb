/**
 * @file db_delete.c
 * @brief pictDB library: do_delete implementation.
 *
 * @author Vincenzo Bazzucchi
 * @author Nicolas Phan Van
 * @date 9 Apr 2016
 */

#include "pictDB.h"

/**
 * @brief Finds the index of an image in an array of metadata.
 *
 * @param pict_id The identifier of the image to be found.
 * @param images  The array of metadata.
 * @param db_size The length of the array.
 * @param index   Pointer to the variable where to write the index of the image.
 * @return 0 if no error occurs, an error coded in error.h in case of error.
 */
int index_of_image(const char* pict_id, const struct pict_metadata* images,
                   const uint32_t db_size, uint32_t* index);


int do_delete(struct pictdb_file* db_file, const char* pict_id)
{
    if (db_file == NULL || pict_id == NULL) {
        return ERR_INVALID_ARGUMENT;
    }
    if (strlen(pict_id) == 0 || strlen(pict_id) > MAX_PIC_ID) {
        return ERR_INVALID_PICID;
    }

    // Find index of image to remove
    uint32_t index;
    if (db_file->header.num_files == 0
        || index_of_image(pict_id, db_file->metadata,
                          db_file->header.max_files, &index) != 0) {
        return ERR_FILE_NOT_FOUND;
    }

    // Mark the image as invalid
    db_file->metadata[index].is_valid = EMPTY;
    // Position write head after the header
    int seek_success = fseek(db_file->fpdb,
                             sizeof(struct pictdb_header)
                             + sizeof(struct pict_metadata) * index,
                             SEEK_SET);

    if (seek_success == 0) {
        // Write metadata
        size_t write_success = fwrite(&db_file->metadata[index],
                                      sizeof(struct pict_metadata),
                                      1, db_file->fpdb);

        if (write_success == 1) {
            // Update header
            ++db_file->header.db_version;
            --db_file->header.num_files;
            // Position write head at beginning of file
            seek_success = fseek(db_file->fpdb, 0, SEEK_SET);

            if (seek_success == 0) {
                // Write header
                write_success = fwrite(&db_file->header,
                                       sizeof(struct pictdb_header),
                                       1, db_file->fpdb);
                return write_success == 1 ? 0 : ERR_IO;
            }
        }
    }

    // Return error code if any of the seek or write checks fails
    return ERR_IO;
}

int index_of_image(const char* pict_id, const struct pict_metadata* images,
                   const uint32_t db_size, uint32_t* index)
{
    for (uint32_t i = 0; i < db_size; ++i) {
        if (images[i].is_valid == NON_EMPTY
            && strcmp(pict_id, images[i].pict_id) == 0) {
            *index = i;
            return 0;
        }
    }

    return ERR_FILE_NOT_FOUND;
}
