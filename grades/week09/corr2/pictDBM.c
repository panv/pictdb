/**
 * @file pictDBM.c
 * @brief pictDB Manager: command line interpretor for pictDB core commands.
 *
 * Picture Database Management Tool
 *
 * @date 2 Nov 2015
 */

#include "pictDB.h"
#include "pictDBM_tools.h"
#include "image_content.h"

// Constants
#define NB_CMD        6   // Number of command line functions the database possesses
#define FILE_DEFAULT  10  // Default max file number
#define THUMB_DEFAULT 64  // Default thumb resolution
#define THUMB_MAX     128 // Maximal thumb resolution
#define SMALL_DEFAULT 256 // Default small resolution
#define SMALL_MAX     512 // Maximal small resolution

// Macro that checks the number of arguments of a command
#define ARG_CHECK(args, min) \
    if (args < min) return ERR_NOT_ENOUGH_ARGUMENTS
// Macro that checks the number of arguments of a create option
#define OPTION_ARG_CHECK(args, expected) \
    if (check_argument_number(args, expected) != 0) \
        return ERR_NOT_ENOUGH_ARGUMENTS
// Macro that checks that resolutions are valid
#define RES_CHECK(x_res, y_res, max) \
    if (check_values(x_res, y_res, max) != 0) \
        return ERR_RESOLUTIONS
// Initializes a new empty database
#define NEW_DATABASE \
    struct pictdb_file db_file = {.fpdb = NULL, .metadata = NULL}

/**
 * @brief A pointer to a function returning an int and taking an int and an
 *        array of pointers to char as parameters.
 */
typedef int (*command)(int, char**);

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

/**
 * @enum options
 * @brief Specifies the create options.
 */
enum options {
    INVALID_OPTION, MAX_FILES, THUMB_RES, SMALL_RES
};

/**
 * @brief Parses the command line options of do_create_cmd.
 *
 * @param option The option to parse.
 * @return A non-zero int corresponding to the option,
 *         or 0 if the argument is not a valid option.
 */
int parse_create_options(const char* option);

/**
 * @brief Checks if there is enough arguments remaining for an option.
 *
 * @param remaining The number of remaining arguments.
 * @param expected  The number of expected arguments for the option.
 * @return 0 if there is enough arguments remaining, 1 otherwise.
 */
int check_argument_number(const int remaining, const int expected);

/**
 * @brief Checks whether the given width and height are within
 *        the acceptable values (0 and max_value).
 *
 * @param x_res     The width.
 * @param y_res     The height.
 * @param max_value The maximum value.
 * @return 0 if the values are valid, 1 otherwise.
 */
int check_values(const uint16_t x_res, const uint16_t y_res,
                 const uint16_t max_value);

/**
 * @brief Reads an image from disk and stores it to a buffer.
 *
 * @param filename     The name of the image file.
 * @param image_buffer The destination of the image.
 * @param image_size   The size of the image.
 * @return 0 in case of success, a non zero error code otherwise.
 */
int read_image_from_disk(const char* filename, char** image_buffer,
                         size_t* image_size);

/**
 * @brief Creates the filename corresponding to the given resolution.
 *
 * @param pict_id    The ID of the image.
 * @param resolution The resolution of the image.
 * @return The new name, or NULL if an error occurred.
 */
char* create_name(const char* pict_id, int resolution);

/**
 * @brief Appends a suffix to the given ID.
 *
 * @param pict_id The ID of the image.
 * @param suffix  The suffix to append to the ID.
 * @param len     The length of the suffix, including the null terminating byte.
 * @return The concatenated string, or NULL if an error occurred.
 */
char* append_suffix(const char* pict_id, const char* suffix, size_t len);

/**
 * @brief Writes the image contained in the given buffer to disk.
 *
 * @param filename     The name of the image file.
 * @param image_buffer The image.
 * @param image_size   The size of the image.
 * @return 0 in case of success, a non zero error code otherwise.
 */
int write_image_to_disk(const char* filename, char* image_buffer,
                        size_t image_size);


/********************************************************************/ /**
 * Opens pictDB file and calls do_list command.
 ********************************************************************** */
int do_list_cmd(int args, char* argv[])
{
    ARG_CHECK(args, 2);

    NEW_DATABASE;

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

    // Default values
    uint32_t max_files = FILE_DEFAULT;
    uint16_t x_thumb_res = THUMB_DEFAULT;
    uint16_t y_thumb_res = THUMB_DEFAULT;
    uint16_t x_small_res = SMALL_DEFAULT;
    uint16_t y_small_res = SMALL_DEFAULT;

    // For each command line argument, checks if there are enough arguments
    // remaining, converts the arguments to integers and assigns them to the
    // variable, and then checks if the variable is valid.
    while (args > 0) {
        switch (parse_create_options(argv[0])) {
        case MAX_FILES:
            OPTION_ARG_CHECK(args, 2);
            max_files = atouint32(argv[1]);
            if (max_files == 0 || max_files > MAX_MAX_FILES) {
                return ERR_MAX_FILES;
            }
            args -= 2;
            argv += 2;
            break;
        case THUMB_RES:
            OPTION_ARG_CHECK(args, 3);
            x_thumb_res = atouint16(argv[1]);
            y_thumb_res = atouint16(argv[2]);
            RES_CHECK(x_thumb_res, y_thumb_res, THUMB_MAX);
            args -= 3;
            argv += 3;
            break;
        case SMALL_RES:
            OPTION_ARG_CHECK(args, 3);
            x_small_res = atouint16(argv[1]);
            y_small_res = atouint16(argv[2]);
            RES_CHECK(x_small_res, y_small_res, SMALL_MAX);
            args -= 3;
            argv += 3;
            break;
        case INVALID_OPTION:
            return ERR_INVALID_ARGUMENT;
        }
    }

    puts("Create");

    // Initialize header and database
    struct pictdb_header db_header = {
        .max_files = max_files,
        .res_resized = { x_thumb_res, y_thumb_res, x_small_res, y_small_res }
    };
    struct pictdb_file db_file = {
        .fpdb = NULL, .header = db_header, .metadata = NULL
    };

    int ret = do_create(filename, &db_file);
    if (ret == 0) {
        print_header(&db_file.header);
    }
    do_close(&db_file);

    return ret;
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
           "  read   <dbfilename> <pictID> [original|orig|thumbnail|thumb|small]:\n"
           "      read an image from the pictDB and save it to a file.\n"
           "      default resolution is \"original\".\n"
           "  insert <dbfilename> <pictID> <filename>: insert a new image in the pictDB.\n"
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

    NEW_DATABASE;

    int ret = do_open(argv[1], "rb+", &db_file);
    if (ret == 0) {
        puts("Delete");
        ret = do_delete(&db_file, argv[2]);
    }
    do_close(&db_file);

    return ret;
}

/********************************************************************/ /**
 * Inserts an image into a database.
 ********************************************************************** */
int do_insert_cmd(int args, char* argv[])
{
    ARG_CHECK(args, 4);

    NEW_DATABASE;

    int ret = do_open(argv[1], "rb+", &db_file);
    if (ret == 0) {
        ret = db_file.header.num_files < db_file.header.max_files ? 0 :
              ERR_FULL_DATABASE;
    }
    if (ret == 0) {
        // Store the image read from disk into a buffer
        char* image_buffer = NULL;
        size_t image_size = 0;
        ret = read_image_from_disk(argv[3], &image_buffer, &image_size);
        if (ret == 0) {
            // Inserts the image into the database
            puts("Insert");
            ret = do_insert(image_buffer, image_size, argv[2], &db_file);
        }
        free(image_buffer);
    }
    do_close(&db_file);

    return ret;
}

/********************************************************************/ /**
 * Reads an image from a database and writes it to disk.
 ********************************************************************** */
int do_read_cmd(int args, char* argv[])
{
    ARG_CHECK(args, 3);

    NEW_DATABASE;
    // Default resolution is RES_ORIG if no optional argument is provided
    int resolution = args > 3 ? resolution_atoi(argv[3]) : RES_ORIG;

    // Open the database only if the resolution is valid
    int ret = resolution != -1 ? do_open(argv[1], "rb+", &db_file) :
              ERR_INVALID_ARGUMENT;
    if (ret == 0) {
        // Store the image read from the database into a buffer
        char* image_buffer = NULL;
        uint32_t image_size = 0;
        puts("Read");
        ret = do_read(argv[2], resolution, &image_buffer, &image_size, &db_file);
        if (ret == 0) {
            char* filename = NULL;
            // Create filename by appending the suffix corresponding to
            // the resolution to the pic ID and write the image to disk
            ret = (filename = create_name(argv[2],
                                          resolution)) == NULL ? ERR_OUT_OF_MEMORY : 0;
            ret = ret == 0 ? write_image_to_disk(filename, image_buffer, image_size) : ret;
            free(filename);
        }
        free(image_buffer);
    }
    do_close(&db_file);

    return ret;
}

/********************************************************************/ /**
 * MAIN
 */
int main(int argc, char* argv[])
{
    if (VIPS_INIT(argv[0])) {
        vips_error_exit("Unable to start VIPS");
    }
    int ret = 0;

    // Map from function name to its pointer
    command_mapping commands[NB_CMD] = {
        { "list", do_list_cmd },
        { "create", do_create_cmd },
        { "delete", do_delete_cmd },
        { "help", help },
        { "read", do_read_cmd },
        { "insert", do_insert_cmd }
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

    vips_shutdown();
    return ret;
}

int parse_create_options(const char* option)
{
    return (strcmp(option, "-max_files") == 0) ? MAX_FILES :
           (strcmp(option, "-thumb_res") == 0) ? THUMB_RES :
           (strcmp(option, "-small_res") == 0) ? SMALL_RES :
           INVALID_OPTION;
}

int check_argument_number(const int remaining, const int expected)
{
    return (remaining < expected) ? 1 : 0;
}

int check_values(const uint16_t x_res, const uint16_t y_res,
                 const uint16_t max_value)
{
    return (x_res == 0 || y_res == 0 || x_res > max_value
            || y_res > max_value) ? 1 : 0;
}

int read_image_from_disk(const char* filename, char** image_buffer,
                         size_t* image_size)
{
    FILE* image = fopen(filename, "rb");
    if (image == NULL) {
        return ERR_IO;
    }
    if (fseek(image, 0, SEEK_END) == 0) {
        *image_size = ftell(image);
        if (fseek(image, 0, SEEK_SET) == 0) {
            *image_buffer = malloc(*image_size);
            if (*image_buffer != NULL) {
                if (fread(*image_buffer, *image_size, 1, image) == 1) {
                    fclose(image);
                    return 0;
                }
            }
        }
    }
    fclose(image);

    return ERR_IO;
}

char* create_name(const char* pict_id, int resolution)
{
    switch (resolution) {
    case RES_THUMB:
        return append_suffix(pict_id, "_thumb.jpg", 11);
    case RES_SMALL:
        return append_suffix(pict_id, "_small.jpg", 11);
    case RES_ORIG:
        return append_suffix(pict_id, "_orig.jpg", 10);
    default:
        return NULL;
    }
}

char* append_suffix(const char* pict_id, const char* suffix, size_t len)
{
    char* new_name = calloc(strlen(pict_id) + len, sizeof(char));
    if (new_name != NULL) {
        strcpy(new_name, pict_id);
        strcat(new_name, suffix);
    }
    return new_name;
}

int write_image_to_disk(const char* filename, char* image_buffer,
                        size_t image_size)
{
    FILE* new_image = fopen(filename, "wb");
    if (new_image == NULL) {
        return ERR_IO;
    }
    int ret = fwrite(image_buffer, image_size, 1, new_image) == 1 ? 0 : ERR_IO;
    fclose(new_image);
    return ret;
}
