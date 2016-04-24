#include "image_content.h"

// Prototypes
int valid_resolution(int resolution);
long write_to_disk(struct pictdb_file* db_file, void* to_write,
                   size_t size, size_t nmemb, long offset, int whence);

void* resize(char* input_buffer, uint32_t input_size, uint16_t max_x, uint16_t max_y);

double shrink_value(VipsImage* image, int max_thumbnail_width, int max_thumbnail_height)
{
    const double h_shrink = (double)max_thumbnail_width / (double)image->Xsize;
    const double v_shrink = (double)max_thumbnail_height / (double)image->Ysize;
    return h_shrink > v_shrink ? v_shrink : h_shrink;
}

int lazily_resize(uint16_t resolution, struct pictdb_file* db_file, size_t index)
{
    // Error checks on arguments
    if (db_file == NULL || index >= db_file->header.max_files) {
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
    if (resolution == RES_ORIG || db_file->metadata[index].size[resolution] == 0) {
        return 0;
    }

    // store in var as used often
    uint32_t size = db_file->metadata[index].size[resolution];

    // Initialize array for image and read it into it.
    //void* image_in_bytes = malloc(size);
    char image_in_bytes[size];

    if (!fseek(db_file->fpdb, size, SEEK_SET)
        || fread(image_in_bytes, size, 1, db_file->fpdb) != size) {
        fprintf(stderr, ERROR_MESSAGES[ERR_IO]);
        return ERR_IO;
    }

    uint16_t index_first_res = resolution * 2;
    void* output_buffer = resize(image_in_bytes, size,
                                 db_file->header.res_resized[index_first_res],
                                 db_file->header.res_resized[index_first_res + 1]);

    if (output_buffer == NULL) {
        fprintf(stderr, ERROR_MESSAGES[ERR_VIPS]);
        return ERR_VIPS;
    }

    // assume db_file.fpdb is open?
    // image = return value of function resizes image
    size_t output_size = sizeof(output_buffer); //ERROR WITH THIS SIZE; ALSO IL NEXT METHOD
                                                // THIS DOESN'T WORK
    long offset = write_to_disk(db_file, output_buffer, output_size, 1, 0, SEEK_END);
    free(output_buffer);

    if (offset != -1) {
        db_file->metadata[index].size[resolution] = output_size;
        db_file->metadata[index].offset[resolution] = offset;
        offset = write_to_disk(db_file, db_file->metadata, sizeof(struct pict_metadata),
                               db_file->header.max_files, sizeof(struct pictdb_header), SEEK_SET);
        return (offset != -1) ? 0 : ERR_IO;
    }

    return ERR_IO;
}

void* resize(char* input_buffer, uint32_t input_size, uint16_t max_x, uint16_t max_y)
{
    // modify image (heavily inspired by thumbify.c)
    VipsObject* process = VIPS_OBJECT(vips_image_new());

    // allocate pointer
    VipsImage** pics = (VipsImage**) vips_object_local_array(process, 2);
    // image_loaded_correctly 0 on success, -1 on error
    int not_loaded = vips_jpegload_buffer(input_buffer, input_size, &pics[0], NULL); //C

    double ratio = shrink_value(pics[0], max_x, max_y); // 0 on success, -1 on error

    int not_resized = vips_resize(pics[0], &pics[1], ratio, NULL);

    // store image back to array
    // 0 on success, -1 on error
    size_t length = sizeof(pics[0]);

    void* output_buffer = malloc(length);
    int not_written = vips_jpegsave_buffer(pics[0], &output_buffer, &length, NULL);
    g_object_unref(process);

    if (not_loaded || not_resized || not_written) {
        return NULL;
    }

    return output_buffer;
}

/**
 * @brief Checks whether the given resolution is within the valid range.
 *
 * @param resolution The resolution to check.
 * @return 0 if the resolution is valid, 1 otherwise
 */
int valid_resolution(int resolution)
{
    return (resolution == RES_THUMB || resolution == RES_SMALL || resolution == RES_ORIG) ? 0 : 1;
}

/**
 * @brief Writes the data pointed to by the given pointer to disk.
 *
 * @param db_file  The database containing the file to write to.
 * @param to_write The pointer pointing to the data to write.
 * @param size     The size in bytes of each element to be written.
 * @param nmemb    The number of elements to be written.
 * @param offset   The offset required by fseek.
 * @param whence   The starting position of the write head.
 * @return -1 in case of error, the size of the file before writing the data otherwise.
 */
long write_to_disk(struct pictdb_file* db_file, void* to_write,
                   size_t size, size_t nmemb, long offset, int whence)
{
    int seek_success = fseek(db_file->fpdb, offset, whence);

    if (seek_success == 0) {
        long file_size = ftell(db_file->fpdb);
        int write_success = fwrite(to_write, size, nmemb, db_file->fpdb);
        return (write_success == size) ? file_size : -1;
    }

    return -1;
}
