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
#include "pictDBM_tools.h"

#define NB_CMD 4 // Number of command line functions the database possesses

typedef int (* command)();

typedef struct {
    const char* command_name;
    command cmd;
} command_mapping;

// Prototypes
int parse_create_options(char* option);
int check_argument_number(int remaining, int expected);
int check_values(uint16_t x_res, uint16_t y_res, uint16_t max_value);


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

    char* filename = argv[1];
    args -= 2;
    argv += 2;

    uint32_t max_files = 0;
    uint16_t x_thumb_res = 0;
    uint16_t y_thumb_res = 0;
    uint16_t x_small_res = 0;
    uint16_t y_small_res = 0;

    for (size_t i = 0; i < args; ++i) {
        switch(parse_create_options(argv[i])) {
            case 1:
                if (check_argument_number(args - (i + 1), 1) != 0) {
                    return ERR_NOT_ENOUGH_ARGUMENTS;
                }
                max_files = atouint32(argv[i + 1]);
                if (max_files == 0 || max_files > MAX_MAX_FILES) {
                    return ERR_MAX_FILES;
                }
                i += 1;
                break;
            case 2:
                if (check_argument_number(args - (i + 1), 2) != 0) {
                    return ERR_NOT_ENOUGH_ARGUMENTS;
                }
                x_thumb_res = atouint16(argv[i + 1]);
                y_thumb_res = atouint16(argv[i + 2]);
                if (check_values(x_thumb_res, y_thumb_res, 128) != 0) {
                    return ERR_RESOLUTIONS;
                }
                i += 2;
                break;
            case 3:
                if (check_argument_number(args - (i + 1), 2) != 0) {
                    return ERR_NOT_ENOUGH_ARGUMENTS;
                }
                x_small_res = atouint16(argv[i + 1]);
                y_small_res = atouint16(argv[i + 2]);
                if (check_values(x_small_res, y_small_res, 512) != 0) {
                    return ERR_RESOLUTIONS;
                }
                i += 2;
                break;
            case 0:
                return ERR_INVALID_ARGUMENT;
        }
    }

    const uint32_t default_max_files = max_files == 0 ? 10 : max_files;
    const uint16_t default_x_thumb_res = x_thumb_res == 0 ? 64 : x_thumb_res;
    const uint16_t default_y_thumb_res = y_thumb_res == 0 ? 64 : y_thumb_res;
    const uint16_t default_x_small_res = x_small_res == 0 ? 256: x_small_res;
    const uint16_t default_y_small_res = y_small_res == 0 ? 256: y_small_res;

    puts("Create");
    struct pictdb_header db_header = {
        .max_files = default_max_files,
        .res_resized = {default_x_thumb_res, default_y_thumb_res,
                        default_x_small_res, default_y_small_res}
    };
    struct pictdb_file db_file = {.header = db_header};

    int db_created = do_create(filename, &db_file);
    if (db_created == 0) {
        print_header(&db_file.header);
    }

    return db_created;
}

int parse_create_options(char* option) {
    return (strcmp(option, "-max_files") == 0) ? 1 :
           (strcmp(option, "-thumb_res") == 0) ? 2 :
           (strcmp(option, "-small_res") == 0) ? 3 :
           0;
}

int check_argument_number(int remaining, int expected) {
    return (remaining < expected) ? 1 : 0;
}

int check_values(uint16_t x_res, uint16_t y_res, uint16_t max_value) {
    return (x_res == 0 || y_res == 0
            || x_res > max_value || y_res > max_value) ? 1 : 0;
}

/********************************************************************//**
 * Displays some explanations.
 ********************************************************************** */
int help (int args, char* argv[])
{
    printf("pictDBM [COMMAND] [ARGUMENTS]\n"
           "  help: displays this help.\n"
           "  list <dbfilename>: list pictDB content.\n"
           "  create <dbfilename> [options]: create a new pictDB.\n"
           "      options are:\n"
           "          -max_files <MAX_FILES>: maximum number of files.\n"
           "                                  default value is 10\n"
           "                                  maximum value is 100000\n"
           "          -thumb_res <X_RES> <Y_RES>: resolution for thumbnail images.\n"
           "                                  default value is 64x64\n"
           "                                  maximum value is 128x128\n"
           "          -small_res <X_RES> <Y_RES>: resolution for small images.\n"
           "                                  default value is 256x256\n"
           "                                  maximum value is 512x512\n"
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
int main(int argc, char* argv[])
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
