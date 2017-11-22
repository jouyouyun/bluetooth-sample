/**
  * Scan nearly bluetooth devices by calling hci methods.
  *
  * Compile: gcc -Wall -g bluetooth_scan.c -lbluetooth
  *
  * Author: jouyouyun <jouyouwen717@gmail.com>
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

int
main(int argc, char *argv[])
{
    inquiry_info *ii = NULL;
    int max_rsp, num_rsp;
    int dev_id, sock, len;
    long flags;
    int i, ret;
    char addr[19] = {0};
    char name[248] = {0};

    // Get the first device
    dev_id = hci_get_route(NULL);
    if (dev_id < 0) {
        fprintf(stderr, "Failed to get device\n");
        return -1;
    }

    sock = hci_open_dev(dev_id);
    if (sock < 0) {
        fprintf(stderr, "Failed to open device(%d)\n", dev_id);
        return -1;
    }

    len = 8;
    max_rsp = 255;
    flags = IREQ_CACHE_FLUSH;
    ii = (inquiry_info*) malloc(max_rsp * sizeof(inquiry_info));
    if (!ii) {
        fprintf(stderr, "Failed to alloc memory\n");
        close(sock);
        return -1;
    }

    for (;;) {
        // Inquiry nearly devices
        num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
        if (num_rsp < 0) {
            fprintf(stderr, "Failed to inquiry\n");
            continue;
        }

        printf("Discovery device number: %d\n", num_rsp);
        // Print nearly bluetooth devices
        for (i = 0; i < num_rsp; i++) {
            memset(addr, 0, sizeof(addr));
            ba2str(&(ii+i)->bdaddr, addr);

            memset(name, 0, sizeof(name));
            ret = hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(name), name, 0);
            if (ret < 0) {
                /* fprintf(stderr, "Failed to get name for '%s'\n", addr); */
                strcpy(name, "[unknown]");
            }

            printf("Addr: %s\nName: %s\n\n", addr, name);
        }
        printf("------------------------------------------\n");
        usleep(500);
    }

    free(ii);
    close(sock);
    return 0;
}
