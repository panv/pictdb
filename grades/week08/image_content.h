/**
 * @file image_content.h
 * @brief Header file for the resize feature.
 *
 */
#include "pictDB.h"

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
