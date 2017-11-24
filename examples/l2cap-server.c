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
#include <bluetooth/l2cap.h>

int
main(int argc, char *argv[])
{
    struct sockaddr_l2 loc_addr = {0}, rem_addr = {0};
    char buf[1024] = {0};
    int sk, client, bytes_read;
    socklen_t opt = sizeof(rem_addr);

    // connect socket
    sk = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
    if (sk < 0) {
        fprintf(stderr, "Failed to connect l2cap socket: %s\n", strerror(errno));
        return -1;
    }

    // bind socket to port 1 of the first available local bluetooth adapter
    loc_addr.l2_family = PF_BLUETOOTH;
    loc_addr.l2_psm = htobs(0x1001);
    bacpy(&loc_addr.l2_bdaddr, BDADDR_ANY);
    int ret = bind(sk, (struct sockaddr*)&loc_addr, sizeof(loc_addr));
    if (ret < 0) {
        fprintf(stderr, "Failed to bind port: %s\n", strerror(errno));
        close(sk);
        return -1;
    }

    ret = listen(sk, 1);
    if (ret != 0) {
        fprintf(stderr, "Failed to listen: %s\n", strerror(errno));
        close(sk);
        return -1;
    }

    client = accept(sk, (struct sockaddr*)&rem_addr, &opt);

    ba2str(&rem_addr.l2_bdaddr, buf);
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
