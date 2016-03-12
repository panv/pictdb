#include <stdio.h>
#include "pictDB.h"

int do_list(const struct pictdb_file db_file) {
	print_header(db_file.header);
	if (db_file.header.num_files == 0) {
		printf("<< empty database >>");
	} else {
		for (size_t i = 0; i < MAX_MAX_FILES; ++i) {
			if (db_file.metadata[i].is_valid == NON_EMPTY)
				print_metadata(db_file.metadata[i]);
		}
	}
}
