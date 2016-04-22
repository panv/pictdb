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
#include "image_content.h"
#include <stdlib.h>
#include <string.h>
#include "error.h"

#define NB_CMD 4 // Number of command line functions the database possesses

typedef int (* command)();

typedef struct {
    const char* command_name;
    command cmd;
} command_mapping;


/********************************************************************//**
 * Opens pictDB file and calls do_list command.
 ********************************************************************** */
int do_list_cmd (int args, char* argv[])
{
    if (args < 2) {
        return ERR_NOT_ENOUGH_ARGUMENTS;
    }

    struct pictdb_file db_file;

    int db_opened = do_open(argv[1], "rb", &db_file);
    if (db_opened == 0) {
        do_list(&db_file);
        do_close(&db_file);
    }
    return db_opened;
}

/********************************************************************//**
 * Prepares and calls do_create command.
********************************************************************** */
int do_create_cmd (int args, char* argv[])
{
    if (args < 2) {
        return ERR_NOT_ENOUGH_ARGUMENTS;
    }

    // This will later come from the parsing of command line arguments
    const uint32_t max_files =  10;
    const uint16_t thumb_res =  64;
    const uint16_t small_res = 256;

    puts("Create");

    struct pictdb_header db_header = {
        .max_files = max_files,
        .res_resized = {thumb_res, thumb_res, small_res, small_res}
    };
    struct pictdb_file db_file = {.header = db_header};

    int db_created = do_create(argv[1], &db_file);
    if (db_created == 0) {
        print_header(&db_file.header);
    }
    return db_created;
}

/********************************************************************//**
 * Displays some explanations.
 ********************************************************************** */
int help (int args, char* argv[])
{
    printf("pictDBM [COMMAND] [ARGUMENTS]\n"
           "  help: displays this help.\n"
           "  list <dbfilename>: list pictDB content.\n"
           "  create <dbfilename>: create a new pictDB.\n"
           "  delete <dbfilename> <pictID>: delete picture pictID from pictDB.\n");
    return 0;
}

/********************************************************************//**
 * Deletes a picture from the database.
 */
int do_delete_cmd (int args, char* argv[])
{
    if (args < 3) {
        return ERR_NOT_ENOUGH_ARGUMENTS;
    }

    struct pictdb_file db_file;

    int db_opened = do_open(argv[1], "rb+", &db_file);
    if (db_opened == 0) {
        puts("Delete");
        int pict_deleted = do_delete(&db_file, argv[2]);
        do_close(&db_file);
        return pict_deleted;
    } else {
        return db_opened;
    }
}

/********************************************************************//**
 * MAIN
 */
int main (int argc, char* argv[])
{
    int ret = 0;

    // Map from function name to its pointer
    command_mapping commands[NB_CMD] = {
        {"list", do_list_cmd},
        {"create", do_create_cmd},
        {"delete", do_delete_cmd},
        {"help", help}
    };

    if (argc < 2) {
        ret = ERR_NOT_ENOUGH_ARGUMENTS;
    } else {
        --argc;
        ++argv; // skips command call name

        int found = 0;
        for (size_t i = 0; i < NB_CMD && found == 0; ++i) {
            if (strcmp(argv[0], commands[i].command_name) == 0) {
                ret = commands[i].cmd(argc, argv);
                found = 1;
            }
        }
        ret = (found != 0) ? ret : ERR_INVALID_COMMAND;
    }

    if (ret) {
        fprintf(stderr, "ERROR: %s\n", ERROR_MESSAGES[ret]);
        (void)help(0, NULL);
    }

    return ret;
}
