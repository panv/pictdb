/**
 * @file db_list.c
 * @brief Implementation of the do_list function.
 *
 * @author Vincenzo Bazzucchi and Nicolas Phan Van
 * @date 12 Mar 2016
*/

#include "pictDB.h"
#include <stdio.h>

/**
 * @brief Prints the metadata of all images of a database.
 *
 * @param db_file In memory structure representing a database.
*/
void do_list(const struct pictdb_file* db_file)
{
    // Print header
    print_header(&db_file->header);

    if (db_file->header.num_files == 0) {
        printf("<< empty database >>\n");
    } else {
        // Iterate over all elements in array of metadata and print them
        // only if they are valid
        for (uint32_t i = 0; i < db_file->header.max_files; ++i) {
            if (db_file->metadata[i].is_valid == NON_EMPTY) {
                print_metadata(&db_file->metadata[i]);
            }
        }
    }
}
