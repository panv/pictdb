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
#include "pictDBM_tools.h"
#include "image_content.h"

// Constants
#define NB_CMD        4 // Number of command line functions the database possesses
#define FILE_DEFAULT  10
#define THUMB_DEFAULT 64
#define THUMB_MAX     128
#define SMALL_DEFAULT 256
#define SMALL_MAX     512

// Macro that checks the number of arguments of a command
#define ARG_CHECK(args, min) \
    if (args < min) return ERR_NOT_ENOUGH_ARGUMENTS;
// Macro that checks the number of arguments of a create option
#define OPTION_ARG_CHECK(args, index, expected) \
    if (check_argument_number(args - (index + 1), expected) != 0) \
        return ERR_NOT_ENOUGH_ARGUMENTS;
// Macro that checks that resolutions are valid
#define RES_CHECK(x_res, y_res, max) \
    if (check_values(x_res, y_res, max) != 0) \
        return ERR_RESOLUTIONS;
// Macro for assigning values to create options
#define ASSIGN_VALUE(value, default_value) \
    value = value == 0 ? default_value : value;

/**
 * @brief A pointer to a function returning an int.
 */
typedef int (*command)();

/**
 * @brief Map from a command name to the function implementing it.
 */
typedef struct {
    /**
     * @brief The command-line name of the function.
     */
    const char* command_name;
    /**
     * @brief The function.
     */
    command cmd;
} command_mapping;

// Prototypes
int parse_create_options(char* option);
int check_argument_number(int remaining, int expected);
int check_values(uint16_t x_res, uint16_t y_res, uint16_t max_value);

/********************************************************************/ /**
 * Opens pictDB file and calls do_list command.
 ********************************************************************** */
int do_list_cmd(int args, char* argv[])
{
    ARG_CHECK(args, 2);

    struct pictdb_file db_file;

    int ret = do_open(argv[1], "rb", &db_file);
    if (ret == 0) {
        do_list(&db_file);
    }
    do_close(&db_file);
    return ret;
}

/********************************************************************/ /**
 * Prepares and calls do_create command.
********************************************************************** */
int do_create_cmd(int args, char* argv[])
{
    ARG_CHECK(args, 2);

    char* filename = argv[1];
    args -= 2;
    argv += 2;

    uint32_t max_files = 0;
    uint16_t x_thumb_res = 0;
    uint16_t y_thumb_res = 0;
    uint16_t x_small_res = 0;
    uint16_t y_small_res = 0;

    // Use of int instead of size_t for the comparison with args
    for (int i = 0; i < args; ++i) {
        switch (parse_create_options(argv[i])) {
        case 1:
            OPTION_ARG_CHECK(args, i, 1);
            max_files = atouint32(argv[i + 1]);
            if (max_files == 0 || max_files > MAX_MAX_FILES) {
                return ERR_MAX_FILES;
            }
            i += 1;
            break;
        case 2:
            OPTION_ARG_CHECK(args, i, 2);
            x_thumb_res = atouint16(argv[i + 1]);
            y_thumb_res = atouint16(argv[i + 2]);
            RES_CHECK(x_thumb_res, y_thumb_res, THUMB_MAX);
            i += 2;
            break;
        case 3:
            OPTION_ARG_CHECK(args, i, 2);
            x_small_res = atouint16(argv[i + 1]);
            y_small_res = atouint16(argv[i + 2]);
            RES_CHECK(x_small_res, y_small_res, SMALL_MAX);
            i += 2;
            break;
        case 0:
            return ERR_INVALID_ARGUMENT;
        }
    }

    ASSIGN_VALUE(max_files, FILE_DEFAULT);
    ASSIGN_VALUE(x_thumb_res, THUMB_DEFAULT);
    ASSIGN_VALUE(y_thumb_res, THUMB_DEFAULT);
    ASSIGN_VALUE(x_small_res, SMALL_DEFAULT);
    ASSIGN_VALUE(y_small_res, SMALL_DEFAULT);

    puts("Create");

    struct pictdb_header db_header = {
        .max_files = max_files,
        .res_resized = { x_thumb_res, y_thumb_res, x_small_res, y_small_res }
    };
    struct pictdb_file db_file = {.header = db_header };

    int ret = do_create(filename, &db_file);
    if (ret == 0) {
        print_header(&db_file.header);
    }
    do_close(&db_file);

    return ret;
}

/**
 * @brief Parses the command line options of do_create_cmd.
 *
 * @param option The option to parse.
 * @return A non-zero int corresponding to the option,
 *         or 0 if the argument is not a valid option.
 */
int parse_create_options(char* option)
{
    return (strcmp(option, "-max_files") == 0) ? 1 :
           (strcmp(option, "-thumb_res") == 0) ? 2 :
           (strcmp(option, "-small_res") == 0) ? 3 : 0;
}

/**
 * @brief Checks if there is enough arguments remaining for an option.
 *
 * @param remaining The number of remaining arguments.
 * @param expected  The number of expected arguments for the option.
 * @return 0 if there is enough arguments remaining, 1 otherwise.
 */
int check_argument_number(int remaining, int expected)
{
    return (remaining < expected) ? 1 : 0;
}

/**
 * @brief Checks whether the given width and height are within
 *        the acceptable values (0 and max_value).
 *
 * @param x_res     The width.
 * @param y_res     The height.
 * @param max_value The maximum value.
 * @return 0 if the values are valid, 1 otherwise.
 */
int check_values(uint16_t x_res, uint16_t y_res, uint16_t max_value)
{
    return (x_res == 0 || y_res == 0 || x_res > max_value
            || y_res > max_value) ? 1 : 0;
}

/********************************************************************/ /**
 * Displays some explanations.
 ********************************************************************** */
int help(int args, char* argv[])
{
    printf("pictDBM [COMMAND] [ARGUMENTS]\n"
           "  help: displays this help.\n"
           "  list <dbfilename>: list pictDB content.\n"
           "  create <dbfilename> [options]: create a new pictDB.\n"
           "      options are:\n"
           "          -max_files <MAX_FILES>: maximum number of files.\n"
           "                                  default value is %d\n"
           "                                  maximum value is %d\n"
           "          -thumb_res <X_RES> <Y_RES>: resolution for thumbnail images.\n"
           "                                  default value is %dx%d\n"
           "                                  maximum value is %dx%d\n"
           "          -small_res <X_RES> <Y_RES>: resolution for small images.\n"
           "                                  default value is %dx%d\n"
           "                                  maximum value is %dx%d\n"
           "  delete <dbfilename> <pictID>: delete picture pictID from pictDB.\n",
           FILE_DEFAULT, MAX_MAX_FILES, THUMB_DEFAULT, THUMB_DEFAULT, THUMB_MAX,
           THUMB_MAX, SMALL_DEFAULT, SMALL_DEFAULT, SMALL_MAX, SMALL_MAX);
    return 0;
}

/********************************************************************/ /**
 * Deletes a picture from the database.
 */
int do_delete_cmd(int args, char* argv[])
{
    ARG_CHECK(args, 3);

    struct pictdb_file db_file;

    int ret = do_open(argv[1], "rb+", &db_file);
    if (ret == 0) {
        puts("Delete");
        ret = do_delete(&db_file, argv[2]);
    }
    do_close(&db_file);
    return ret;
}

/********************************************************************/ /**
 * MAIN
 */
int main(int argc, char* argv[])
{
    int ret = 0;

    // Map from function name to its pointer
    command_mapping commands[NB_CMD] = {
        { "list", do_list_cmd },
        { "create", do_create_cmd },
        { "delete", do_delete_cmd },
        { "help", help }
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
        // Command not found
        ret = (found != 0) ? ret : ERR_INVALID_COMMAND;
    }

    if (ret) {
        fprintf(stderr, "ERROR: %s\n", ERROR_MESSAGES[ret]);
        (void)help(0, NULL);
    }

    return ret;
}
