#include "pictDB.h"
#include <vips/vips.h>

// Prototypes
int valid_resolution(int resolution);

int lazily_resize(int resolution, struct pictdb_file* db_file, size_t index)
{
    // Error checks on arguments
    if (db_file == NULL)   // checker index > max_files?
    {
        return ERR_INVALID_ARGUMENT;
    }
    if (valid_resolution != 0)
    {
        return ERR_RESOLUTIONS;
    }

    // If the image already exists in the asked resolution or the asked resolution
    // is the original resolution, do nothing
    if (resolution == RES_ORIG || db_file->metadata[index].size[resolution] == 0)
    {
        return 0;
    }

    // store in var as used often
    uint32_t size = db_file->metadata[index].size[resolution];

    // Position read head to start of original image
    int seek_success = fseek(db_file->fpdb, size, SEEK_SET);
    // Initialize array for image and read it into it.
    void image_in_bytes[size];
    int read_image = fread(image_in_bytes, size, 1, db_file->fpdb); //check == 1

    // allocate pointer
    VipsImage** original;
    // image_loaded_correctly 0 on success, -1 on error
    int image_loaded_correctly = vips_jpegload_buffer(image_in_bytes, size, original);

    // modify image (heavily inspired by thumbify.c)
    VipsObject* process = VIPS_OBJECT( vips_image_new() );

    VipsImage** resized = (VipsImage**) vips_object_local_array(process, 1);

    uint16_t firstResIndex = resolution * 2;
    double ratio = shrink_value(
            original,
            db_file->header.res_resized[firstResIndex],
            db_file->header.res_resized[firstResIndex + 1]
            );

    // 0 on success, -1 on error
    int could_resize = vips_resize(original, &resized[0], ratio, ratio, NULL);

    // store image back to array
    void outputBuffer[VIPS_IMAGE_SIZEOF_PEL];
    // 0 on success, -1 on error
    int write_to_buffer_ok = vips_jpegsave_buffer(resized[0], outputBuffer, sizeof(resized[0]));



}

int valid_resolution(int resolution)
{
    return (resolution == RES_THUMB || resolution == RES_SMALL ||
            resolution == RES_ORIG) ? 0 : 1;
}
