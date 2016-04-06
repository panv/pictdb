/**
 * @file db_list.c
 * @brief implementation of the do_list function
 *
 * @author Vincenzo Bazzucchi and Nicolas Phan Van
 * @date 12 Mar 2016
*/


#include <stdio.h>
#include "pictDB.h"

void do_list(const struct pictdb_file* db_file)
{
    // Print header
    print_header(&db_file->header);
    if (db_file->header.num_files == 0) {
        printf("<< empty database >>\n");
    } else {
        // iterate over all elements in array of metadata and print them
        // only if they are valid
        for (size_t i = 0; i < MAX_MAX_FILES; ++i) {
            if (db_file->metadata[i].is_valid == NON_EMPTY)
                print_metadata(&db_file->metadata[i]);
        }
    }
}
