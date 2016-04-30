#include "pictDB.h"

int do_insert(struct pictdb_file* db_file)
{
    if (!(db_file->header.num_files < db_file->header.max_files)) {
        return ERR_FULL_DATABASE;
    }

    return 0;

}

