/**
 * @file pictDB_server.c
 * @brief pictDB Manager: webserver version.
 *
 * @author Vincenzo Bazzucchi
 * @author Nicolas Phan Van
 */

#include "pictDB.h"
#include "mongoose.h"

static const char* s_http_port = "8000"; // Port
static struct mg_serve_http_opts s_http_server_opts;
static int s_sig_received = 0;           // Signal


static void handle_list_call(struct mg_connection* nc, struct http_message* hm)
{
    
}

static void signal_handler(int sig_num)
{
    signal(sig_num, signal_handler);
    s_sig_received = sig_num;
}

static void ev_handler(struct mg_connection* nc, int ev, void* ev_data)
{
    struct http_message* hm = (struct http_message*) ev_data;
    
    switch(ev) {
    case MG_EV_HTTP_REQUEST:
        if (mg_vcmp(&hm->uri, "/pictDB/list") == 0) {
            handle_list_call(nc, hm);
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
    int ret = 0;
    
    // Initialize and open database if there is enough arguments
    struct pictdb_file db_file = {.fpdb = NULL, .metadata = NULL};
    ret = argc < 2 ? ERR_NOT_ENOUGH_ARGUMENTS : do_open(argv[1], "rb+", &db_file);
    
    if (ret == 0) {
        print_header(&db_file.header);
        
        struct mg_mgr mgr;
        struct mg_connection* nc;
        
        signal(SIGTERM, signal_handler); // Kill previous signal handler i think
        signal(SIGINT, signal_handler);  // Init new signal handler
        
        mg_mgr_init(&mgr, NULL); // Initialize event manager
        nc = mg_bind(&mgr, s_http_port, ev_handler);
        
        // Set up HTTP server parameters
        mg_set_protocol_http_websocket(nc);
        // s_http_server_opts.document_root = ".";      // Serve current directory
        s_http_server_opts.document_root = argv[0];   // un des deux ^ <
        // s_http_server_opts.dav_document_root = ".";  // Allow access via WebDav pas sur
        s_http_server_opts.enable_directory_listing = "yes";
        
        // Listening loop
        printf("Starting web server on port %s\n", s_http_port);
        while (!s_sig_received) {
            mg_mgr_poll(&mgr, 1000); // 1000000?
        }
        printf("Exiting on signal %d\n", s_sig_received);
        
        mg_mgr_free(&mgr);
    }
    
    do_close(&db_file);
    
    // Print error message if there was an error
    if (ret) {
        fprintf(stderr, "ERROR: %s\n", ERROR_MESSAGES[ret]);
        // (void)help(0, NULL);
    }
    
    return ret;
}
