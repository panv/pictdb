/**
 * @file image_content.c
 * @brief Implements the image resizing feature.
 *
 * @author Vincenzo Bazzucchi
 * @author Nicolas Phan Van
 */

#include "image_content.h"
#include <vips/vips.h>

/**
 * @brief Checks whether the given resolution is within the valid range.
 *
 * @param resolution The resolution to check.
 * @return 0 if the resolution is valid, 1 otherwise
 */
int valid_resolution(int resolution);

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
                   size_t size, size_t nmemb, long offset, int whence);

/**
 * @brief Resizes the given image according to width and height constraints.
 *
 * @param input_buffer The image to resize.
 * @param input_size Size of the image to resize.
 * @param max_x, max_y The maximum width and height of the new image.
 * @param output_size Pointer to the location of the size of the resized image.
 *
 * return The resized image.
 */
void* resize(char* input_buffer, uint32_t input_size, uint16_t max_x,
             uint16_t max_y, size_t* output_size);

/**
 * @brief Computes the ratio to use for resizing.
 *
 * @param image The image to resize.
 * @param max_width, max_height Maximal width and height of the new image.
 * @param dest Memory location where to write the ratio.
 */
void shrink_value(VipsImage* image, uint16_t max_width,
                  uint16_t max_height, double* dest);

int lazily_resize(uint16_t resolution, struct pictdb_file* db_file,
                  size_t index)
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

    uint32_t size = db_file->metadata[index].size[RES_ORIG]; //Used often.

    // Initialize array for image and read it into it.
    char image_in_bytes[size];
    if (!fseek(db_file->fpdb, size, SEEK_SET)
        || fread(image_in_bytes, size, 1, db_file->fpdb) != size) {
        fprintf(stderr, ERROR_MESSAGES[ERR_IO]);
        return ERR_IO;
    }

    // Resize the image using VIPS
    size_t output_size = 0;
    void* output_buffer = resize(image_in_bytes, size,
                                 db_file->header.res_resized[resolution * 2],
                                 db_file->header.res_resized[resolution * 2 + 1],
                                 &output_size);
    if (output_buffer == NULL) {
        g_free(output_buffer);
        fprintf(stderr, ERROR_MESSAGES[ERR_VIPS]);
        return ERR_VIPS;
    }

    long offset = write_to_disk(db_file, output_buffer, output_size, 1, 0,
                                SEEK_END);
    g_free(output_buffer); // Once written, we can free the memory from the image.

    if (offset != -1) {
        db_file->metadata[index].size[resolution] = output_size;
        db_file->metadata[index].offset[resolution] = offset;
        offset = write_to_disk(db_file, db_file->metadata,
                               sizeof(struct pict_metadata),
                               db_file->header.max_files,
                               sizeof(struct pictdb_header), SEEK_SET);
        return (offset != -1) ? 0 : ERR_IO;
    }
    return ERR_IO;
}

void* resize(char* input_buffer, uint32_t input_size, uint16_t max_x,
             uint16_t max_y, size_t* output_size)
{
    VipsObject* process = VIPS_OBJECT(vips_image_new());
    VipsImage** pics = (VipsImage**)vips_object_local_array(process, 2);
    double ratio = 0.0;
    shrink_value(pics[0], max_x, max_y, &ratio);
    void* output_buffer;

    if (vips_jpegload_buffer(input_buffer, input_size, &pics[0], NULL)
        || vips_resize(pics[0], &pics[1], ratio, NULL)
        || vips_jpegsave_buffer(pics[1], &output_buffer, output_size, NULL)) {
        return NULL;
    }

    g_object_unref(process);
    return output_buffer;
}

void shrink_value(VipsImage* image, uint16_t max_width,
                  uint16_t max_height, double* dest)
{
    const double h_shrink = (double)max_width / (double)image->Xsize;
    const double v_shrink = (double)max_height / (double)image->Ysize;
    *dest = h_shrink > v_shrink ? v_shrink : h_shrink;
}

int valid_resolution(int resolution)
{
    return (resolution == RES_THUMB || resolution == RES_SMALL
            || resolution == RES_ORIG) ? 0 : 1;
}

long write_to_disk(struct pictdb_file* db_file, void* to_write,
                   size_t size, size_t nmemb, long offset, int whence)
{
    if (!fseek(db_file->fpdb, offset, whence)) {
        long file_size = ftell(db_file->fpdb);
        size_t write_success = fwrite(to_write, size, nmemb, db_file->fpdb);
        return (write_success == size) ? file_size : -1;
    }
    return -1;
}
