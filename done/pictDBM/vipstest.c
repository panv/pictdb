#include "pictDB.h"
#include <vips/vips.h>

void export_all_pics(struct pictdb_file* dbfile);

int main(void)
{
    return 0;
}

void export_all_pics(struct pictdb_file* dbfile)
{
    for (uint32_t pic = 0; pic < dbfile->header.num_files; ++pic){
        printf("Exporting pic number %d\n", pic);
        for (uint32_t res = 0; res < NB_RES; ++res){
            printf("res: %d\n", res);
            fseek(dbfile->fpdb, dbfile->metadata[pic].offset[res], SEEK_SET);
            puts("fseek OK");
            uint32_t size = dbfile->metadata[pic].size[res];
            char im[size];
            fread(im, size, 1, dbfile->fpdb);
            puts("fread OK");

            VipsObject* process = VIPS_OBJECT(vips_image_new());
            VipsImage** image = (VipsImage**)vips_object_local_array(process, 1);
            vips_jpegload_buffer(im, size, &image[0], NULL);
            char name[] {pic + '0', '-', res + '0'};
            vips_image_write_to_file(image[0], name, NULL);
            g_object_unref(process);
        }
    }

}
