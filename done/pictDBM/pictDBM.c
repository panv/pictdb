/**
 * @file pictDBM.c
 * @brief pictDB Manager: command line interpretor for pictDB core commands.
 *
 * Picture Database Management Tool
 *
 * @author Mia Primorac
 * @date 2 Nov 2015
 */

#include "pictDB.h"
#include <stdlib.h>
#include <string.h>
#include "error.h"

/********************************************************************//**
 * Opens pictDB file and calls do_list command.
 ********************************************************************** */
int do_list_cmd (const char* filename) {
    struct pictdb_file db_file;
    
    int db_opened = do_open(filename, "rb", &db_file);
    if (db_opened == 0) {
        do_list(&db_file);
        do_close(&db_file);
    }
    return db_opened;

    /* This is a quick and dirty way of reading the file.
     * It's provided here as such to avoid solution leak.
     * You shall NOT proceed as such in your future open function
     * (in week 6).
     */
    /* **********************************************************************
     * TODO WEEK 06: REPLACE THE PROVIDED CODE BY YOUR OWN CODE HERE
     * **********************************************************************
     */
    /*
    myfile.fpdb = fopen(filename, "rb");
    if (myfile.fpdb == NULL) {
        return ERR_IO;
    }
    fread(&myfile.header , sizeof(struct pictdb_header),             1, myfile.fpdb);
    fread(myfile.metadata, sizeof(struct pict_metadata), MAX_MAX_FILES, myfile.fpdb);
    */
}

/********************************************************************//**
 * Prepares and calls do_create command.
********************************************************************** */
int do_create_cmd (const char* filename) {
    // This will later come from the parsing of command line arguments
    const uint32_t max_files =  10;
    const uint16_t thumb_res =  64;
    const uint16_t small_res = 256;

    puts("Create");
    /* **********************************************************************
     * TODO WEEK 05: WRITE YOUR CODE HERE (and change the return if needed).
     * **********************************************************************
     */

    struct pictdb_header db_header = {
        .max_files = max_files,
        .res_resized = {thumb_res, thumb_res, small_res, small_res}
    };
    struct pictdb_file db_file = {.header = db_header};

    int db_created = do_create(filename, &db_file);
    if (db_created == 0) {
        print_header(&db_file.header);
    }
    return db_created;
}

/********************************************************************//**
 * Displays some explanations.
 ********************************************************************** */
int help (void) {
    /* **********************************************************************
     * TODO WEEK 05: WRITE YOUR CODE HERE (and change the return if needed).
     * **********************************************************************
     */
    printf("pictDBM [COMMAND] [ARGUMENTS]\n"
           "  help: displays this help.\n"
           "  list <dbfilename>: list pictDB content.\n"
           "  create <dbfilename>: create a new pictDB.\n"
           "  delete <dbfilename> <pictID>: delete picture pictID from pictDB.\n");
    return 0;
    // should we return one of the errors defined in error.h? INVALID_COMMAND?
}

/********************************************************************//**
 * Deletes a picture from the database.
 */
int do_delete_cmd (const char* filename, const char* pictID) {
    // No test on filename, do_open will take care of it
    if (pictID == NULL || strlen(pictID) > MAX_PIC_ID) {
        return ERR_INVALID_PICID;
    }
    
    struct pictdb_file db_file;
    
    int db_opened = do_open(filename, "wb", &db_file);
    if (db_opened == 0) {
        puts("Delete");
        int pict_deleted = do_delete(&db_file, pictID);
        do_close(&db_file);
        return pict_deleted;
    } else {
        return db_opened;
    }
}

/********************************************************************//**
 * MAIN
 */
int main (int argc, char* argv[]) {
    int ret = 0;

    if (argc < 2) {
        ret = ERR_NOT_ENOUGH_ARGUMENTS;
    } else {
        /* **********************************************************************
         * TODO WEEK 08: THIS PART SHALL BE REVISED THEN (WEEK 09) EXTENDED.
         * **********************************************************************
         */
        argc--; argv++; // skips command call name
        if (!strcmp("list", argv[0])) {
            if (argc < 2) {
                ret = ERR_NOT_ENOUGH_ARGUMENTS;
            } else {
                ret = do_list_cmd(argv[1]);
            }
        } else if (!strcmp("create", argv[0])) {
            if (argc < 2) {
                ret = ERR_NOT_ENOUGH_ARGUMENTS;
            } else {
                ret = do_create_cmd(argv[1]);
            }
        } else if (!strcmp("delete", argv[0])) {
            if (argc < 3) {
                ret = ERR_NOT_ENOUGH_ARGUMENTS;
            } else {
                ret = do_delete_cmd(argv[1], argv[2]);
            }
        } else if (!strcmp("help", argv[0])) {
            ret = help();
        } else {
            ret = ERR_INVALID_COMMAND;
        }
    }

    if (ret) {
        fprintf(stderr, "ERROR: %s\n", ERROR_MESSAGES[ret]);
        (void)help();
    }

    return ret;
}
