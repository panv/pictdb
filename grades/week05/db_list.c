/**
 * @file db_list.c
 * @brief Implementation of the do_list function.
 *
 * @date 12 Mar 2016
*/

#include <stdio.h>
#include "pictDB.h"

/**
 * @brief Prints the metadata of all images of a database.
 *
 * @param db_file In memory structure representing a database
*/
void do_list(const struct pictdb_file db_file)
{
    print_header(db_file.header);
    if (db_file.header.num_files == 0) {
        printf("<< empty database >>\n");
    } else {
        for (uint32_t i = 0; i < db_file.header.max_files; ++i) {
            if (db_file.metadata[i].is_valid == NON_EMPTY) {
                print_metadata(db_file.metadata[i]);
            }
        }
    }
}
