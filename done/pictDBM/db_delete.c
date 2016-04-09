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

/*
 * @brief Checks for equality between two strings
 *
 * @param s1, s2 The two strings to compare.
 *
 * @return A non-zero value if the strings are equal, else 0.
 */
int equal_string(const char* s1, const char* s2);


int do_delete(struct pictdb_file* db_file, const char* pict_id)
{
    if (db_file == NULL || pict_id == NULL){
        return ERR_INVALID_ARGUMENT;
    }

    // Find index of image to remove
    uint32_t index;
    int found = index_of_image(pict_id, db_file->metadata,
            db_file->header.num_files, &index);
    if (found == ERR_FILE_NOT_FOUND) {
        return ERR_FILE_NOT_FOUND;
    }

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
                write_success = fwrite(db_file->header, sizeof(pictdb_header), 1, db_file->fpdb);
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
        if (equal_string(pict_id, images[i].pict_id) != 0) {
            *index = i;
            return 0;
        }
    }
    return ERR_FILE_NOT_FOUND;
}

int equal_string(const char* s1, const char* s2)
{
    int are_equal = strlen(s1) == strlen(s2);
    for (size_t i = 0; i < strlen(s1) && are_equal != 0; ++i) {
        are_equal = s1[i] == s2[i];
    }
    return are_equal;
    /*
    int are_equal = 1;
    for (size_t i = 0; i < strlen(s1); ++i){
        are_equal = (s1[i] == s2[i]) && are_equal;
    }
    return are_equal && (strlen(s1) == strlen(s2));*/
}
