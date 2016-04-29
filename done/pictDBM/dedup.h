/**
 * @file dedup.h
 * @brief Header file for deduplication.
 *
 * @author Vincenzo Bazzucchi
 * @author Nicolas Phan Van
 */
#include "pictDB.h"

/**
 * @brief Deduplicates the image at index i.
 *
 * @param db_file The database file.
 * @param index The index of the image.
 *
 * @return 0 If no error occurred, else an error code defined in error.h
 */
int do_name_and_content_dedup(struct pictdb_file* db_file, uint32_t index);
