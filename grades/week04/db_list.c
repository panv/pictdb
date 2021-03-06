/**
 * @file db_list.c
 * @brief implementation of the do_list function
 *
 * @date 12 Mar 2016
*/


#include <stdio.h>
#include "pictDB.h"

void do_list(const struct pictdb_file db_file) {
	print_header(db_file.header);
	if (db_file.header.num_files == 0) {
		printf("<< empty database >>\n");
	} else {
		for (size_t i = 0; i < MAX_MAX_FILES; ++i) { //correcteur: utilisez plutôt db_file.header.max_files
			if (db_file.metadata[i].is_valid == NON_EMPTY)
				print_metadata(db_file.metadata[i]); //correcteur: privilegiez l'utilisation de {} même si la condition ne contient qu'une seule ligne
		}
	}
}
