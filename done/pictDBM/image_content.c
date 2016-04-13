#include pictDB.h

// Prototypes
int valid_resolution(int resolution);
long write_to_disk(struct pictdb_file* db_file,
                   void* to_write, long offset, int whence);


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
    long offset = write_to_disk(db_file, image, 0, SEEK_END);
    if (offset != -1) {
        db_file->metadata[index].size[resolution] = sizeof(image);
        metadata->offset[resolution] = offset;
        offset = write_to_disk(db_file, db_file->metadata,
                               sizeof(struct pictdb_header), SEEK_SET);
        return (offset != -1) ? 0 : ERR_IO;
    }
    return ERR_IO;
}

/**
 * @brief Checks whether the given resolution is within the valid range.
 *
 * @param resolution The resolution to check.
 * @return 0 if the resolution is valid, 1 otherwise
 */
int valid_resolution(int resolution) {
    return (resolution == RES_THUMB || resolution == RES_SMALL ||
            resolution == RES_ORIG) ? 0 : 1;
}

/**
 * @brief Writes the data pointed to by the given pointer to disk.
 *
 * @param db_file The database containing the file to write to.
 * @param to_write The pointer pointing to the data to write.
 * @param offset The offset required by fseek.
 * @param whence The starting position of the write head.
 * @return -1 in case of error, the size of the file before writing the data otherwise.
 */
long write_to_disk(struct pictdb_file* db_file,
                   void* to_write, long offset, int whence) {
    int seek_success = fseek(db_file->fpdb, offset, whence);
    if (seek_success == 0) {
        long offset = ftell(db_file->fpdb);
        int write_success = fwrite(to_write, sizeof(*to_write), 1, db_file->fpdb);
        return (write_success == 1) ? offset : -1;
    }
    return -1;
}
