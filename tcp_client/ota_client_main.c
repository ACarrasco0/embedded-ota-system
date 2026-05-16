#include "ota_client.h"
#include <stdio.h>
#include <string.h>

#define DEFAULT_PORT "4050"

static void print_usage(const char *program)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s <ip> <port> <firmware_path>\n", program);
    fprintf(stderr, "  %s --host <ip> [--port <port>] --file <firmware_path>\n", program);
}

int main(int argc, char *argv[])
{
    const char *host = NULL;
    const char *port = DEFAULT_PORT;
    const char *path = NULL;

    if (argc == 4) {
        host = argv[1];
        port = argv[2];
        path = argv[3];
    } else {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--host") == 0 && i + 1 < argc) {
                host = argv[++i];
            } else if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
                port = argv[++i];
            } else if (strcmp(argv[i], "--file") == 0 && i + 1 < argc) {
                path = argv[++i];
            } else {
                print_usage(argv[0]);
                return 1;
            }
        }
    }

    if (host == NULL || path == NULL) {
        print_usage(argv[0]);
        return 1;
    }

    if (ota_client_send_update(host, port, path) != 0) {
        fprintf(stderr, "OTA update failed\n");
        return 1;
    }

    printf("OTA update completed successfully\n");
    return 0;
}  
 
