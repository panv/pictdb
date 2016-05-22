/**
 * @file db_list.c
 * @brief Implementation of the do_list function.
 *
 * @author Vincenzo Bazzucchi
 * @author Nicolas Phan Van
 * @date 12 Mar 2016
 */

#include "pictDB.h"
#include <json-c/json.h>

/**
 * @brief Displays a database on stdout.
 *
 * @param db_file The database to be displayed.
 */
void do_list_cmd_line(const struct pictdb_file* db_file);

/**
 * @brief Stringify a database using the json syntax.
 *
 * The resulting string will have the following structure:
 * {
 * "Pictures": [] # an array of the strings of the pict_id fields from the metadata
 * }
 *
 * @param db_file The database to stringify.
 * @return The string representing the database.
 */
const char* do_list_web(const struct pictdb_file* db_file);


const char* do_list(const struct pictdb_file* db_file, enum do_list_mode mode)
{
    switch(mode) {
    case STDOUT:
        do_list_cmd_line(db_file);
        return NULL;
    case JSON:
        return do_list_web(db_file);
    default:
        // Unrecognized mode
        return "Unimplemented do_list mode";
    }
}

void do_list_cmd_line(const struct pictdb_file* db_file)
{
    // Print header
    print_header(&db_file->header);

    if (db_file->header.num_files == 0) {
        printf("<< empty database >>\n");
    } else {
        // Iterate over all elements in array of metadata and print them
        // only if they are valid
        for (uint32_t i = 0; i < db_file->header.max_files; ++i) {
            if (db_file->metadata[i].is_valid == NON_EMPTY) {
                print_metadata(&db_file->metadata[i]);
            }
        }
    }
}

const char* do_list_web(const struct pictdb_file* db_file)
{
    // JSON array that will contain the pict_id
    struct json_object* pictid_array = json_object_new_array();
    // Add each valid image pict_id to the array
    for (uint32_t i = 0; i < db_file->header.max_files; ++i) {
        if (db_file->metadata[i].is_valid == NON_EMPTY) {
            struct json_object* pictid = json_object_new_string(db_file->metadata[i].pict_id);
            json_object_array_add(pictid_array, pictid);
        }
    }
    // Create the JSON object that contains the array of pict_id
    struct json_object* wrapper = json_object_new_object();
    json_object_object_add(wrapper, "Pictures", pictid_array);
    // Convert the wrapper to string
    const char* ret = json_object_to_json_string(wrapper);
    // Free the wrapper - does it free the array too??
    json_object_put(wrapper);
    return ret;
}
