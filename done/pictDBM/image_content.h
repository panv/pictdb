/**
 * @file image_content.h
 * @brief Header file for the resize feature.
 *
 * @author Vincenzo Bazzucchi
 * @author Nicolas Phan Van
 */

#ifndef PICTDBPRJ_PICTDB_H
#include "pictDB.h"
#endif
#include <vips/vips.h>

/**
 * @brief Resizes an image to a new resolution.
 *
 * @param resolution The resolution of the image to create. Possible values are
 *                   RES_THUMB, RES_SMALL, RES_ORIG.
 * @param db_file    The database file containing the image to resize
 *                   and the resized image.
 * @param index      The index of the image to resize in the database.
 * @return 0 if the resize was successful, a non-zero int otherwise.
 */
int lazily_resize(int resolution, struct pictdb_file* db_file,
                  size_t index);

/**
 * @brief Retrieves the resolution (width and height) of an image.
 *
 * @param height       Location where the height of the image will be stored.
 * @param width        Location where the width of the image will be stored.
 * @param image_buffer Pointer to the memory location of the image.
 * @param image_size   The size of the image (in bytes).
 * @return 0 if the operation was successful, ERR_VIPS otherwise.
 */
int get_resolution(uint32_t* height, uint32_t* width, const char* image_buffer,
                   size_t image_size);
