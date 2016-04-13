#include pictDB.h

// Prototypes
int valid_resolution(int resolution);

int lazily_resize(int resolution, struct pictdb_file* db_file, size_t index) {
    // Error checks on arguments
    if (db_file == NULL || index >= db_file->max_files) {
        return ERR_INVALID_ARGUMENT;
    }
    if (db_file->metadata[index].is_valid == EMPTY) {
        printf("Error : image not contained in the database");
        return ERR_INVALID_ARGUMENT;
    }
    if (valid_resolution(resolution) != 0) {
        return ERR_RESOLUTIONS;
    }

    // If the image already exists in the asked resolution or the asked resolution
    // is the original resolution, do nothing
    if (resolution == RES_ORIG ||
        db_file->metadata[index].size[resolution] == 0) {
        return 0;
    }

    // assume db_file.fpdb is open?
    // image = return value of function resizes image
    long offset = write_to_disk(db_file, image);
    if (offset != -1) {
        db_file->metadata[index].size[resolution] = sizeof(image);
        metadata->offset[resolution] = offset;

    }
    return ERR_IO;
}

int valid_resolution(int resolution) {
    return (resolution == RES_THUMB || resolution == RES_SMALL ||
            resolution == RES_ORIG) ? 0 : 1;
}

long write_to_disk(struct pictdb_file* db_file, void* to_write) {
    int seek_success = fseek(db_file->fpdb, 0, SEEK_END);
    if (seek_success == 0) {
        long offset = ftell(db_file->fpdb);
        int write_success = fwrite(to_write, sizeof(*to_write), 1, db_file->fpdb);
        return (write_success == 1) ? offset : -1;
    }
    return -1;
}
