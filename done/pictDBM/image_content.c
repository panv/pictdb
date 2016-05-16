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
 * @return 0 if the resolution is valid, 1 otherwise.
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
 * @return -1 in case of error, the offset of the written data in the database
 *         file otherwise.
 */
long write_to_disk(struct pictdb_file* db_file, void* to_write,
                   size_t size, size_t nmemb, long offset, int whence);

/**
 * @brief Updates the metadata at the given index with the new size and offset,
 *        and writes it to disk.
 *
 * @param db_file    The database containing the metadata.
 * @param index      The index of the metadata.
 * @param resolution The resolution of the resized image.
 * @param size       The size of the resized image.
 * @param offset     The offset of the resized image.
 * @return -1 in case of error, a non negative long otherwise.
 */
long update_metadata(struct pictdb_file* db_file, size_t index, int resolution,
                     size_t size, size_t offset);

/**
 * @brief Resizes the given image according to width and height constraints.
 *
 * @param output_buffer Destination of the resized image.
 * @param output_size   Pointer to the location of the size of the resized image.
 * @param input_buffer  The image to resize.
 * @param input_size    The size (in bytes) of the image to resize.
 * @param max_x         The maximum width of the new image.
 * @param max_y         The maximum height of the new image.
 * @return 0 in case of success, 1 otherwise.
 */
int resize(void** output_buffer, size_t* output_size, const void* input_buffer,
           uint32_t input_size, uint16_t max_x, uint16_t max_y);

/**
 * @brief Computes the ratio to use for resizing.
 *
 * @param image      The image to resize.
 * @param max_width  The maximum width of the new image.
 * @param max_height The maximum height of the new image.
 * @return The resize ratio.
 */
double shrink_value(VipsImage* image, uint16_t max_width, uint16_t max_height);


int lazily_resize(int resolution, struct pictdb_file* db_file,
                  size_t index)
{
    // Error checks on arguments
    if (valid_resolution(resolution) != 0 || db_file == NULL
        || index >= db_file->header.max_files) {
        return ERR_INVALID_ARGUMENT;
    }
    if (db_file->header.num_files == 0
        || db_file->metadata[index].is_valid == EMPTY) {
        fprintf(stderr, "Error : image not contained in the database");
        return ERR_INVALID_ARGUMENT;
    }

    // Used often
    struct pict_metadata* meta_index = &db_file->metadata[index];

    // If the image already exists in the asked resolution or the asked
    // resolution is the original resolution, do nothing.
    if (resolution == RES_ORIG
        || meta_index->size[resolution] != 0) {
        return 0;
    }

    uint32_t size_orig = meta_index->size[RES_ORIG]; // Used often

    // Store the original image in an array
    char image_in_bytes[size_orig];
    if (fseek(db_file->fpdb, meta_index->offset[RES_ORIG], SEEK_SET)
        || fread(image_in_bytes, size_orig, 1, db_file->fpdb) != 1) {
        return ERR_IO;
    }

    // Resize the image using VIPS
    size_t output_size = 0;
    void* output_buffer = NULL;
    if (resize(&output_buffer, &output_size,
               image_in_bytes, size_orig,
               db_file->header.res_resized[resolution * 2],
               db_file->header.res_resized[resolution * 2 + 1]) != 0) {
        return ERR_VIPS;
    }

    // Write the image at the end of the file and get the offset
    long file_position = write_to_disk(db_file, output_buffer, output_size,
                                       1, 0, SEEK_END);
    // Once written, we can free the memory from the image
    g_free(output_buffer);
    if (file_position != -1) {
        // Update the metadata and write it to disk
        file_position = update_metadata(db_file, index, resolution,
                                        output_size, file_position);
        // Update the metadata of the duplicate images, if there is any
        for (size_t i = 0; i < db_file->header.max_files && file_position != -1; ++i) {
            if (i != index && db_file->metadata[i].is_valid == NON_EMPTY) {
                if (hashcmp(db_file->metadata[i].SHA, meta_index->SHA) == 0) {
                    file_position = update_metadata(db_file, i, resolution,
                            output_size, meta_index->offset[resolution]);
                }
            }
        }
        if (file_position != -1) {
            // Update the header and write it to disk
            ++db_file->header.db_version;
            file_position = write_to_disk(db_file, &db_file->header,
                                          sizeof(struct pictdb_header), 1,
                                          0, SEEK_SET);
            return file_position == -1 ? ERR_IO : 0;
        }
    }
    return ERR_IO;
}

int get_resolution(uint32_t* height, uint32_t* width, const char* image_buffer,
                   size_t image_size)
{
    VipsObject* process = VIPS_OBJECT(vips_image_new());
    VipsImage** workspace = (VipsImage**) vips_object_local_array(process, 1);
    if (vips_jpegload_buffer((void*) image_buffer, image_size,
                             &workspace[0], NULL)) {
        g_object_unref(process);
        return ERR_VIPS;
    }
    *height = workspace[0]->Ysize;
    *width = workspace[0]->Xsize;
    g_object_unref(process);
    //g_free(workspace[0]);
    return 0;
}

int valid_resolution(int resolution)
{
    return (resolution == RES_THUMB || resolution == RES_SMALL
            || resolution == RES_ORIG) ? 0 : 1;
}

long write_to_disk(struct pictdb_file* db_file, void* to_write,
                   size_t size, size_t nmemb, long offset, int whence)
{
    if (fseek(db_file->fpdb, offset, whence) == 0) {
        long file_position = ftell(db_file->fpdb);
        size_t write_success = fwrite(to_write, size, nmemb, db_file->fpdb);
        return (write_success == nmemb) ? file_position : -1;
    }
    return -1;
}

long update_metadata(struct pictdb_file* db_file, size_t index, int resolution,
                     size_t size, size_t offset) {
    db_file->metadata[index].size[resolution] = size;
    db_file->metadata[index].offset[resolution] = offset;
    return write_to_disk(db_file, &db_file->metadata[index],
                         sizeof(struct pict_metadata), 1, sizeof(struct pictdb_header)
                         + sizeof(struct pict_metadata) * index, SEEK_SET);
}

int resize(void** output_buffer, size_t* output_size, const void* input_buffer,
           uint32_t input_size, uint16_t max_x, uint16_t max_y)
{
    VipsObject* process = VIPS_OBJECT(vips_image_new());
    VipsImage** workspace = (VipsImage**) vips_object_local_array(process, 2);
    if (vips_jpegload_buffer((void*) input_buffer, input_size,
                             &workspace[0], NULL) != 0) {
        g_object_unref(process);
        return 1;
    }
    int ret = 0;
    double ratio = shrink_value(workspace[0], max_x, max_y);
    if (vips_resize(workspace[0], &workspace[1], ratio, NULL) ||
        vips_jpegsave_buffer(workspace[1], output_buffer, output_size, NULL)) {
        ret = 1;
    }
    g_object_unref(process);
    //g_free(workspace[0]);
    //g_free(workspace[1]);
    return ret;
}

double shrink_value(VipsImage* image, uint16_t max_width, uint16_t max_height)
{
    const double h_shrink = (double)max_width / (double)image->Xsize;
    const double v_shrink = (double)max_height / (double)image->Ysize;
    return h_shrink > v_shrink ? v_shrink : h_shrink;
}
