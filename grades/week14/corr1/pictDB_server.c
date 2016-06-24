/**
 * @file pictDB_server.c
 * @brief pictDB Manager: webserver version.
 *
 */

#include "pictDB.h"
#include "libmongoose/mongoose.h"
#include <vips/vips.h>
#include "html_msg.h"

#define MAX_QUERY_PARAM 5

// Image database - defined as a global variable to facilitate its use
// in the different call handlers
struct pictdb_file* db_file;
const char* s_http_port = "8000"; // Listening port
struct mg_serve_http_opts s_http_server_opts;
int s_sig_received = 0;           // Signal

/**
 * @brief Initializes an empty pictdb_file.
 *
 * @param argc The number of command line arguments passed to the program.
 *             Used for checking if the dbfile has been passed to the server.
 * @param filename The filename of the database file.
 * @return 0 if the deletion was successful, an error code otherwise.
 */
int init_dbfile(int argc, const char* filename);

/**
 * @brief Serves a list request.
 *
 * @param nc The Network Connection used to communicate.
 */
void handle_list_call(struct mg_connection* nc);

/**
 * @brief Serves a read request.
 *
 * @param nc The Network Connection used to communicate.
 * @param hm The HTTP message containing information about the image to read.
 */
void handle_read_call(struct mg_connection* nc, struct http_message* hm);

/**
 * @brief Serves an insert request.
 *
 * @param nc The Network Connection used to communicate.
 * @param hm The HTTP message containing information about the image to read.
 */
void handle_insert_call(struct mg_connection* nc,
                        struct http_message* hm);

/**
 * @brief Serves a delete request.
 *
 * @param nc The Network Connection used to communicate.
 * @param hm The HTTP message containing information about the image to read.
 */
void handle_delete_call(struct mg_connection* nc,
                        struct http_message* hm);

/**
 * @brief Parses the array containing the uri split results.
 *
 * @param result     The array containing the result of the split method.
 * @param resolution The pointer to stock the value of the 'res' uri tag to.
 * @param pict_id    The pointer to stock the picture id to.
 */
void parse_uri(char* result[], int* resolution, char** pict_id);

/**
 * @brief Sends error messages to clients.
 *
 * @param nc The Network Connection used to communicate.
 * @param int The error code as defined in error.h
 */
void mg_error(struct mg_connection* nc, int error);

/**
 * @brief Stores the signal identifier.
 *
 * @param sig_num The code of the received signal.
 */
void signal_handler(int sig_num);

/**
 * @brief Calls the event handler corresponding to the received event.
 *
 * @param nc The Network Connection used to communicate.
 * @param ev The event code.
 * @param ev_data Information about the event.
 */
void db_event_handler(struct mg_connection* nc, int ev, void* ev_data);


int init_dbfile(int argc, const char* filename)
{
    db_file = malloc(sizeof(struct pictdb_file));
    if (db_file != NULL) {
        db_file->fpdb = NULL;
        db_file->metadata = NULL;
        return argc < 2 ? ERR_NOT_ENOUGH_ARGUMENTS : do_open(filename, "rb+", db_file);
    }
    return ERR_OUT_OF_MEMORY;
}

void mg_error(struct mg_connection* nc, int error)
{
    size_t total = strlen(error_start) + strlen(error_end) + strlen(
                       ERROR_MESSAGES[error]) + 3;
    mg_printf(nc,
              "HTTP/1.1 500\r\n"
              "Content-Type: text/html\r\n"
              "Content-Length: %zu\r\n\r\n%s%s%s",
              total, error_start, ERROR_MESSAGES[error], error_end);
    nc->flags |= MG_F_SEND_AND_CLOSE;
}

void handle_list_call(struct mg_connection* nc)
{
    char* json_list = do_list(db_file, JSON);
    if (json_list == NULL) {
        mg_error(nc, ERR_IO);
        return;
    }
    size_t msg_length = strlen(json_list);
    mg_printf(nc,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/json\r\n"
              "Content-Length: %zu\r\n\r\n%s",
              msg_length, json_list);
    nc->flags |= MG_F_SEND_AND_CLOSE;
    free(json_list);
}

void parse_uri(char* result[], int* resolution, char** pict_id)
{
    size_t i = 0;
    while (i < MAX_QUERY_PARAM && result[i] != NULL) {
        if (strcmp(result[i], "res") == 0) {
            *resolution = resolution_atoi(result[i + 1]);
            i += 2;
        } else if (strcmp(result[i], "pict_id") == 0) {
            char* id = calloc(strlen(result[i + 1]) + 1, sizeof(char));
            int decoded = mg_url_decode(result[i + 1], strlen(result[i + 1]), id,
                                        MAX_PIC_ID, 0);
            if (decoded == -1) {
                free(id);
                *pict_id = NULL;
                return;
            }
            *pict_id = id;
            i += 2;
        } else {
            ++i;
        }
    }
}

void handle_read_call(struct mg_connection* nc, struct http_message* hm)
{
    char** result = init_result_array(MAX_QUERY_PARAM);
    char* tmp = init_tmp((MAX_PIC_ID + 1) * MAX_QUERY_PARAM);
    if (result == NULL || tmp == NULL) {
        free(result);
        free(tmp);
        mg_error(nc, ERR_OUT_OF_MEMORY);
        return;
    }

    split(result, tmp, hm->query_string.p, "&=", hm->query_string.len,
          MAX_QUERY_PARAM);

    int resolution = -1;
    char* pict_id = NULL;
    parse_uri(result, &resolution, &pict_id);

    if (resolution != -1 && pict_id != NULL) {
        char* image_buffer = NULL;
        uint32_t image_size = 0;
        int err_check = do_read(pict_id, resolution, &image_buffer,
                                &image_size, db_file);
        free(pict_id);
        if (err_check != 0) {
            mg_error(nc, err_check);
        } else {
            mg_printf(nc,
                      "HTTP/1.1 200 OK\r\n"
                      "Content-Type: image/jpeg\r\n"
                      "Content-Length: %" PRIu32 "\r\n\r\n",
                      image_size);
            mg_send(nc, image_buffer, image_size);
            nc->flags |= MG_F_SEND_AND_CLOSE;
        }
        free(image_buffer);
    } else {
        mg_error(nc, ERR_INVALID_ARGUMENT);
    }

    free(result);
    free(tmp);
}

void handle_insert_call(struct mg_connection* nc,
                        struct http_message* hm)
{
    int err_check = db_file->header.num_files < db_file->header.max_files ? 0 :
                    ERR_FULL_DATABASE;

    if (err_check == 0) {
        char var_name[100];
        char file_name[MAX_PIC_ID + 1];
        const char* chunk;
        size_t chunk_len = 0;
        size_t n1 = 0;
        size_t n2 = 0;

        while ((n2 = mg_parse_multipart(hm->body.p + n1,
                                        hm->body.len - n1,
                                        var_name, sizeof(var_name),
                                        file_name, sizeof(file_name),
                                        &chunk, &chunk_len)) > 0) {
            printf("var: %s, file_name: %s, size: %zu, chunk: [%.*s]\n",
                   var_name, file_name, chunk_len, (int) chunk_len, chunk);
            n1 += n2;
        }

        err_check = do_insert(chunk, chunk_len, file_name, db_file);

        // Success
        if (err_check == 0) {
            mg_printf(nc,
                      "HTTP/1.1 302 Found\r\n"
                      "Location: http://localhost:%s/index.html\r\n",
                      s_http_port);
            nc->flags |= MG_F_SEND_AND_CLOSE;
        }
    }

    if (err_check != 0) {
        mg_error(nc, err_check);
    }
}

void handle_delete_call(struct mg_connection* nc,
                        struct http_message* hm)
{
    char** result = init_result_array(MAX_QUERY_PARAM);
    char* tmp = init_tmp((MAX_PIC_ID + 1) * MAX_QUERY_PARAM);
    if (result == NULL || tmp == NULL) {
        free(result);
        free(tmp);
        mg_error(nc, ERR_OUT_OF_MEMORY);
        return;
    }

    split(result, tmp, hm->query_string.p, "&=", hm->query_string.len,
          MAX_QUERY_PARAM);

    int err_check = -1;
    char* pict_id = NULL;
    parse_uri(result, &err_check, &pict_id);

    if (pict_id != NULL) {
        err_check = do_delete(db_file, pict_id);
        if (err_check == 0) {
            mg_printf(nc,
                      "HTTP/1.1 302 Found\r\n"
                      "Location: http://localhost:%s/index.html\r\n",
                      s_http_port);
            nc->flags |= MG_F_SEND_AND_CLOSE;
        }
    }

    if (err_check != 0) {
        mg_error(nc, err_check);
    }

    free(result);
    free(tmp);
}

void signal_handler(int sig_num)
{
    signal(sig_num, signal_handler);
    s_sig_received = sig_num;
}

void db_event_handler(struct mg_connection* nc, int ev, void* ev_data)
{
    struct http_message* hm = (struct http_message*) ev_data;

    switch (ev) {
    case MG_EV_HTTP_REQUEST:
        if (mg_vcmp(&hm->uri, "/pictDB/list") == 0) {
            handle_list_call(nc);
        } else if (mg_vcmp(&hm->uri, "/pictDB/read") == 0) {
            handle_read_call(nc, hm);
        } else if (mg_vcmp(&hm->uri, "/pictDB/insert") == 0) {
            handle_insert_call(nc, hm);
        } else if (mg_vcmp(&hm->uri, "/pictDB/delete") == 0) {
            handle_delete_call(nc, hm);
        } else {
            mg_serve_http(nc, hm, s_http_server_opts); // Serve static content
        }
        break;
    default:
        break;
    }
}

int main(int argc, char* argv[])
{
    if (VIPS_INIT(argv[0])) {
        vips_error_exit("Unable to start VIPS");
    }

    int ret = 0;

    // Initialize and open database
    ret = init_dbfile(argc, argv[1]);

    if (ret == 0) {
        print_header(&db_file->header);

        // Initialize signal handler and kill previous one
        signal(SIGTERM, signal_handler);
        signal(SIGINT, signal_handler);

        struct mg_mgr mgr;
        struct mg_connection* nc;

        // Create listening connection
        mg_mgr_init(&mgr, NULL);
        nc = mg_bind(&mgr, s_http_port, db_event_handler);

        if (nc != NULL) {
            // Set up HTTP server parameters
            mg_set_protocol_http_websocket(nc);
            s_http_server_opts.document_root = "."; // Serve current directory
            s_http_server_opts.enable_directory_listing = "yes";

            // Listening loop
            printf("Starting web server on port %s,\nserving %s\n", s_http_port,
                   s_http_server_opts.document_root);
            while (!s_sig_received) {
                mg_mgr_poll(&mgr, 1000);
            }
            printf("Exiting on signal %d\n", s_sig_received);
        } else {
            fprintf(stderr, "Unable to create web server on port %s\n", s_http_port);
        }

        mg_mgr_free(&mgr);
    }

    // Close database and free the pointer
    do_close(db_file);
    free(db_file);

    // Print error message if there was an error
    if (ret) {
        fprintf(stderr, "ERROR: %s\n", ERROR_MESSAGES[ret]);
    }

    vips_shutdown();

    return ret;
}
