#include "pictDB.h"
#include "image_content.h"
#include <vips/vips.h>

void export_all_pics(struct pictdb_file* dbfile)
{
    for (uint32_t pic = 0; pic < dbfile->header.num_files; ++pic) {
        printf("Exporting pic number %d\n", pic);
        for (uint32_t res = 0; res < NB_RES; ++res) {
            if (res != 2) {
                lazily_resize(res, dbfile, pic);
            }
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
            char name[7] = {pic + '0', '-', res + '0', '.', 'j', 'p', 'g'};
            vips_jpegsave(image[0], name, NULL);
            g_object_unref(process);
        }
    }
}

size_t get_index(struct pictdb_file* db_file, const char* picID)
{
    for (size_t i = 0; i < db_file->header.max_files; ++i) {
        if (db_file->metadata[i].is_valid == NON_EMPTY
            && strcmp(picID, db_file->metadata[i].pict_id) == 0) {
            return i;
        }
    }
    return 0;
}

int main(int argc, char* argv[])
{
    --argc;
    ++argv;

    struct pictdb_file db_file;
    do_open(argv[0], "rb+", &db_file);
    export_all_pics(&db_file);
    /*
        size_t index = get_index(&db_file, argv[1]);
        lazily_resize(RES_THUMB, &db_file, index);
        lazily_resize(RES_SMALL, &db_file, index);
    */
    do_list(&db_file);
    do_close(&db_file);

    return 0;
}
