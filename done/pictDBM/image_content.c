#include pictDB.h

// Prototypes
int valid_resolution(int resolution);

int lazily_resize(int resolution, struct pictdb_file* db_file, size_t index) {
    // Error checks on arguments
    if (db_file == NULL) { // checker index > max_files?
        return ERR_INVALID_ARGUMENT;
    }
    if (valid_resolution != 0) {
        return ERR_RESOLUTIONS;
    }

    // If the image already exists in the asked resolution or the asked resolution
    // is the original resolution, do nothing
    if (resolution == RES_ORIG || db_file->metadata[index].size[resolution] == 0) {
        return 0;
    } 
}

int valid_resolution(int resolution) {
    return (resolution == RES_THUMB || resolution == RES_SMALL ||
            resolution == RES_ORIG) ? 0 : 1;
}
