/**
 * Operate bluetooth devices by calling ioctl methods.
 * To get device list, connected devices, device UP/DOWN
 *
 * Compile: gcc -Wall -g rfcomm-server.c -lbluetooth
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
dynamic_bind_rc(int sk, struct sockaddr_rc *sockaddr, uint8_t *port)
{
    int err;
    for (*port = 1; *port <= 31; (*port)++) {
        sockaddr->rc_channel = *port;
        err = bind(sk, (struct sockaddr*)sockaddr, sizeof(sockaddr));
        if (!err || errno == EINVAL) {
            break;
        }
    }

    if (*port == 31) {
        err = -1;
        errno = EINVAL;
    }
    return err;
}

int
main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <bluetooth address>\n", argv[0]);
        return -1;
    }

    struct sockaddr_rc loc_addr = {0}, rem_addr = {0};
    char buf[1024] = {0};
    int sk, client, bytes_read;
    socklen_t opt = sizeof(rem_addr);

    // connect socket
    sk = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (sk < 0) {
        fprintf(stderr, "Failed to connect rfcomm socket: %s\n", strerror(errno));
        return -1;
    }

    // bind socket to port 1 of the first available local bluetooth adapter
    loc_addr.rc_family = AF_BLUETOOTH;
    int ret = str2ba(argv[1], &loc_addr.rc_bdaddr);
    if (ret != 0) {
        fprintf(stderr, "Failed to convert bluetooth address: %s\n", strerror(errno));
        close(sk);
        return -1;
    }

    uint8_t port;
    ret = dynamic_bind_rc(sk, &loc_addr, &port);
    if (ret < 0) {
        fprintf(stderr, "Failed to bind port: %s\n", strerror(errno));
        close(sk);
        return -1;
    }
    printf("Success to bind on %u\n", port);

    ret = listen(sk, 1);
    if (ret != 0) {
        fprintf(stderr, "Failed to listen: %s\n", strerror(errno));
        close(sk);
        return -1;
    }

    client = accept(sk, (struct sockaddr*)&rem_addr, &opt);

    ba2str(&rem_addr.rc_bdaddr, buf);
    printf("Accepted connection from %s\n", buf);
    memset(buf, 0, sizeof(buf));

    bytes_read = read(client, buf, sizeof(buf));
    if (bytes_read > 0) {
        printf("Received [%s]\n", buf);
    }

    close(client);
    close(sk);
    return 0;
}
