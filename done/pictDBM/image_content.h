#include "pictDB.h"
#include <stdlib.h>
#include <vips/vips.h>

int lazily_resize(uint16_t resolution, struct pictdb_file* db_file, size_t index);
