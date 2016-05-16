/**
 * @file dedup.h
 * @brief Header file for deduplication.
 *
 * @author Vincenzo Bazzucchi
 * @author Nicolas Phan Van
 */

#ifndef PICTDBPRJ_PICTDB_H
#include "pictDB.h"
#endif

/**
 * @brief Deduplicates the image at the given index.
 *
 * @param db_file The database containing the image.
 * @param index   The index of the image.
 * @return 0 if no error occurred, an error code defined in error.h otherwise.
 */
int do_name_and_content_dedup(struct pictdb_file* db_file, uint32_t index);
