/**
 * Operate bluetooth devices by calling ioctl methods.
 * To get device list, connected devices, device UP/DOWN
 *
 * Compile: gcc -Wall -g rfcomm-client.c -lbluetooth
 *
 * Author: jouyouyun <jouyouwen717@gmail.com>
 **/

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

int
main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <bluetooth address>\n", argv[0]);
        return -1;
    }

    struct sockaddr_rc addr = {0};
    int sk, status;

    sk = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (sk < 0) {
        fprintf(stderr, "Failed to connect rfcomm socket: %s\n", strerror(errno));
        return -1;
    }

    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = (uint8_t)1;
    int ret = str2ba(argv[1], &addr.rc_bdaddr);
    if (ret != 0) {
        fprintf(stderr, "Failed to convert bluetooth address: %s\n", strerror(errno));
        close(sk);
        return -1;
    }

    status = connect(sk, (struct sockaddr*)&addr, sizeof(addr));
    if (status == 0) {
        status = write(sk, "Hello, world!", 13);
    }

    if (status < 0) {
        fprintf(stderr, "Failed to connect/write: %s\n", strerror(errno));
    }

    close(sk);
    return 0;
}
