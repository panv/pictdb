/**
 * @file pictDB_server.c
 * @brief pictDB Manager: webserver version.
 *
 * @author Vincenzo Bazzucchi
 * @author Nicolas Phan Van
 */

#include "pictDB.h"
#include "mongoose.h"

#define MAX_QUERY_PARAM 5

static const char* s_http_port = "8000"; // Port
static struct mg_serve_http_opts s_http_server_opts;
static int s_sig_received = 0;           // Signal
static struct pictdb_file* db_file;


static int init_dbfile(int argc, const char* filename)
{
    db_file = malloc(sizeof(struct pictdb_file));
    if (db_file != NULL) {
        db_file->fpdb = NULL;
        db_file->metadata = NULL;
        return argc < 2 ? ERR_NOT_ENOUGH_ARGUMENTS : do_open(filename, "rb+", db_file);
    }
    return ERR_OUT_OF_MEMORY;
}

static void handle_list_call(struct mg_connection* nc)
{
    char* json_list = do_list(db_file, JSON);
    size_t msg_length = strlen(json_list);
    mg_printf(nc,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/json\r\n"
              "Content-Length: %zu\r\n\r\n%s",
              msg_length, json_list);
    nc->flags |= MG_F_SEND_AND_CLOSE;
    free(json_list);
}

static void split(char* result[], char* tmp, const char* src,
                  const char* delim, size_t len)
{
    strncpy(tmp, src, len);
    tmp[len] = '\0';

    size_t i = 0;
    while ((tokens = strtok(tmp, delim)) != NULL) {
        result[i] = tmp;
        ++i;
        tmp = NULL;
    }
}

static void handle_read_call(struct mg_connection* nc, struct http_message* hm)
{
    char* result[MAX_QUERY_PARAM];
    size_t max_length = (MAX_PIC_ID + 1) * MAX_QUERY_PARAM;
    char* tmp = calloc(max_length, sizeof(char));
    if (tmp == NULL) {
        mg_error(nc, ERR_OUT_OF_MEMORY);
        return;
    }
    tmp[max_length] = '\0';

    split(result, tmp, hm->query_string.p, "&=", hm->query_string.len);

    int resolution = -1;
    char* pict_id = NULL;
    size_t i = 0;
    while (i < MAX_QUERY_PARAM && result[i] != NULL) {
        if (strcmp(result[i], "res") == 0) {
            resolution = resolution_atoi(result[i + 1]);
            i += 2;
        } else if (strcmp(result[i], "pict_id") == 0) {
            pict_id = result[i + 1];
            i += 2;
        } else {
            ++i;
        }
    }

    if (resolution != -1 && pict_id != NULL) {
        char* image_buffer = NULL;
        uint32_t image_size = 0;
        int err_check = do_read(pict_id, resolution, &image_buffer,
                                &image_size, db_file);
        if (err_check != 0) {
            mg_error(nc, err_check);
        } else {
            mg_printf(nc,
                      "HTTP/1.1 200 OK\r\n"
                      "Content-Type: image/jpeg\r\n",
                      "Content-Length: %" PRIu32 "\r\n\r\n",
                      image_size);
            mg_send(nc, image_buffer, image_size); // checker ce truc?
            nc->flags |= MG_F_SEND_AND_CLOSE;
        }
        free(image_buffer);
    } else {
        mg_error(nc, ERR_INVALID_ARGUMENT);
    }

    free(tmp);
}

static void mg_error(struct mg_connection* nc, int error)
{
    mg_printf(nc,
              "HTTP/1.1 500\r\n"
              "Content-Length: %d\r\n\r\n%s",
              0, ERROR_MESSAGES[error]);
    nc->flags |= MG_F_SEND_AND_CLOSE;
}

static void signal_handler(int sig_num)
{
    signal(sig_num, signal_handler);
    s_sig_received = sig_num;
}

static void db_event_handler(struct mg_connection* nc, int ev, void* ev_data)
{
    struct http_message* hm = (struct http_message*) ev_data;

    switch (ev) {
    case MG_EV_HTTP_REQUEST:
        if (mg_vcmp(&hm->uri, "/pictDB/list") == 0) {
            handle_list_call(nc);
        } else if (mg_vcmp(&hm->uri, "/pictDB/read") == 0) {
            handle_read_call(nc, hm);
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

    // Initialize and open database if there is enough arguments
    ret = init_dbfile(argc, argv[1]);

    if (ret == 0) {
        print_header(&db_file->header);

        struct mg_mgr mgr;
        struct mg_connection* nc;

        signal(SIGTERM, signal_handler); // Kill previous signal handler i think
        signal(SIGINT, signal_handler);  // Init new signal handler

        mg_mgr_init(&mgr, NULL); // Initialize event manager
        nc = mg_bind(&mgr, s_http_port, db_event_handler);

        // Set up HTTP server parameters
        mg_set_protocol_http_websocket(nc);
        // s_http_server_opts.document_root = ".";      // Serve current directory
        s_http_server_opts.document_root = argv[0];   // un des deux ^ <
        // s_http_server_opts.dav_document_root = ".";  // Allow access via WebDav pas sur
        s_http_server_opts.enable_directory_listing = "yes";

        // Listening loop
        printf("Starting web server on port %s\n, serving %s\n", s_http_port,
               s_http_server_opts.document_root);
        while (!s_sig_received) {
            mg_mgr_poll(&mgr, 1000); // 1000000?
        }
        printf("Exiting on signal %d\n", s_sig_received);

        mg_mgr_free(&mgr);
    }

    do_close(db_file);
    free(db_file);

    // Print error message if there was an error
    if (ret) {
        fprintf(stderr, "ERROR: %s\n", ERROR_MESSAGES[ret]);
        // (void)help(0, NULL);
    }

    vips_shutdown();

    return ret;
}
