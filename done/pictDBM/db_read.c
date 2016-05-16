/**
 * @file db_read.c
 * @brief Allows the read of a picture from a database.
 *
 * @author Vincenzo Bazzucchi
 * @author Nicolas Phan Van
 */
#ifndef PICTDBPRJ_PICTDB_H
#include "pictDB.h"
#endif
#include "image_content.h"

/**
 * Déjà commenté dans pictdb.h!!
 *
 * @brief Reads the image with given id exporting it, in the requested
 * resolution, to a memory buffer.
 *
 * @param pict_id The id of the image to read.
 * @param resolution The resolution (defined in pictDBM.c) to use
 * when exporting.
 * @param image_buffer Pointer to memory location where to export the
 * image.
 * @param image_size The size (in bytes) of the image destination buffer,
 * established and affected by this function.
 * @param db_file The database file to read from.
 *
 * @return An error code coded in error.h.
 */
int do_read(const char* pict_id, int resolution, char** image_buffer,
            uint32_t* image_size, struct pictdb_file* db_file)
{
    // Parameter verification.
    if (pict_id == NULL || db_file == NULL) {
        return ERR_INVALID_ARGUMENT;
    }
    if (strlen(pict_id) == 0 || strlen(pict_id) > MAX_PIC_ID) {
        return ERR_INVALID_PICID;
    }
    if (db_file->header.num_files == 0) {
        return ERR_FILE_NOT_FOUND;
    }

    // Look for the image to extract in the metadata array.
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

    struct pict_metadata* to_read = &db_file->metadata[idx]; // Convenience

    // If the resolution is not original, and the asked one is not
    // in the database, generate it.
    if (resolution != RES_ORIG && (to_read->offset[resolution] == 0 ||
                                   to_read->size[resolution] == 0)) {
        ret = lazily_resize(resolution, db_file, idx);
        if (ret != 0) {
            return ret;
        }
    }
    // Prepare memory destination of the image.
    *image_buffer = malloc(to_read->size[resolution]);
    if (*image_buffer == NULL) {
        return ERR_OUT_OF_MEMORY;
    }
    // Move read head and read image from disk.
    if (fseek(db_file->fpdb, to_read->offset[resolution], SEEK_SET) ||
        fread(*image_buffer, to_read->size[resolution], 1,
              db_file->fpdb) != 1) {
        free(*image_buffer); //In case of IO error, free unused memory.
        return ERR_IO;
    }
    *image_size = to_read->size[resolution]; // Set size.
    return 0;
}
