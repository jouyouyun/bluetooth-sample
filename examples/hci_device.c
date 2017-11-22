/**
 * Operate bluetooth devices by calling ioctl methods.
 * To get device list, connected devices, device UP/DOWN
 *
 * Compile: gcc -Wall -g hci_devices.c -lbluetooth
 *
 * Author: jouyouyun <jouyouwen717@gmail.com>
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/ioctl.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

#define INQUIRY_MAX_RSP_NUM 255

int
query_conn_list(int sk, int dev_id)
{
    struct hci_conn_list_req *cl = NULL;
    struct hci_conn_info *ci = NULL;

    cl = (struct hci_conn_list_req*)malloc(HCI_MAX_DEV*sizeof(*cl) + sizeof(*cl));
    if (!cl) {
        fprintf(stderr, "Failed to dev(%d) connected list: %s\n", dev_id, strerror(errno));
        return -1;
    }

    memset(cl, 0, HCI_MAX_DEV*sizeof(*cl) + sizeof(*cl));
    cl->dev_id = dev_id;
    cl->conn_num = HCI_MAX_DEV;
    ci = cl->conn_info;

    int ret = ioctl(sk, HCIGETCONNLIST, (void*)cl);
    if (ret < 0) {
        fprintf(stderr, "Failed to get dev(%d) connected list: %s\n", dev_id, strerror(errno));
        free(cl);
        return -1;
    }

    printf("\tDevice(%d), connected number: %d\n", dev_id, cl->conn_num);
    int i = 0;
    char addr[19] = {0};
    for (; i < cl->conn_num; i++, ci++) {
        memset(addr, 0, 19);
        ba2str(&ci->bdaddr, addr);
        printf("\tConnected Info: %s\n", addr);
        printf("\t\tHandler: %u\n", ci->handle);
        printf("\t\tType: %u\n", ci->type);
        printf("\t\tOut: %u\n", ci->out);
        printf("\t\tState: %u\n", ci->state);
        printf("\t\tLink Mode: %u\n", ci->link_mode);
        printf("\n");
    }

    free(cl);
    return 0;
}

int
query_dev_info(int sk, int dev_id)
{
    struct hci_dev_info di;
    char addr[19] = {0};
    int i = 0, ret = 0;

    di.dev_id = dev_id;
    ret = ioctl(sk, HCIGETDEVINFO, (void*)&di);
    if (ret < 0) {
        fprintf(stderr, "Failed to get dev(%d) info: %s\n", dev_id, strerror(errno));
        return -1;
    }

    printf("'%s' info:\n", di.name);
    printf("\tId: %d\n", di.dev_id);
    ba2str(&di.bdaddr, addr);
    printf("\tAddress: %s\n", addr);
    printf("\tFlags: %u\n", di.flags);
    printf("\tType: %u\n", di.type);
    printf("\tFeatures:");
    for (i = 0; i < 8; i++) {
        printf("%u, ", di.features[i]);
    }
    printf("\n");
    printf("\tPKT Type: %u\n", di.pkt_type);
    printf("\tLink Policy: %u\n", di.link_policy);
    printf("\tLink Mode: %u\n", di.link_mode);
    printf("\tACL MTU: %u\n", di.acl_mtu);
    printf("\tACL PKTS: %u\n", di.acl_pkts);
    printf("\tSCO MTU: %u\n", di.sco_mtu);
    printf("\tSCO PKTS: %u\n", di.sco_pkts);
    printf("\tStat:\n");
    printf("\t\tError RX: %u\n", di.stat.err_rx);
    printf("\t\tError TX: %u\n", di.stat.err_tx);
    printf("\t\tCommand TX: %u\n", di.stat.cmd_tx);
    printf("\t\tEvent Type RX: %u\n", di.stat.evt_rx);
    printf("\t\tACL RX: %u\n", di.stat.acl_rx);
    printf("\t\tACL TX: %u\n", di.stat.acl_tx);
    printf("\t\tSCO RX: %u\n", di.stat.sco_rx);
    printf("\t\tSCO TX: %u\n", di.stat.sco_tx);
    printf("\t\tByte RX: %u\n", di.stat.byte_rx);
    printf("\t\tByte TX: %u\n", di.stat.byte_tx);
    printf("\n");

    query_conn_list(sk, dev_id);
    printf("\n\n");
    return 0;
}

int
scan_nearly_dev(int sk, int dev_id)
{
    struct hci_inquiry_req *ir = NULL;
    inquiry_info *ii = NULL;
    void *buf = NULL;

    buf = malloc(sizeof(*ir)+sizeof(inquiry_info)*INQUIRY_MAX_RSP_NUM);
    if (!buf) {
        fprintf(stderr, "Failed to alloc memory for inquiry: %s\n", strerror(errno));
        return -1;
    }

    // try 5 times if no remote device found
    int times = 0;
    for (; times < 5; times++) {
        usleep(500);

        printf("Do scanning, the %d times\n", times);
        memset(buf, 0, sizeof(*ir)+sizeof(inquiry_info)*INQUIRY_MAX_RSP_NUM);
        ir = buf;
        ir->dev_id = dev_id;
        ir->num_rsp = INQUIRY_MAX_RSP_NUM;
        ir->length = 8;
        ir->flags = IREQ_CACHE_FLUSH;
        // Why?
        ir->lap[0] = 0x33;
        ir->lap[1] = 0x8b;
        ir->lap[2] = 0x9e;

        int ret = ioctl(sk, HCIINQUIRY, (unsigned long)buf);
        if (ret < 0) {
            fprintf(stderr, "Failed to inquiry dev(%d): %s\n", dev_id, strerror(errno));
            /* free(buf); */
            /* return -1; */
            continue;
        }

        if (ir->num_rsp < 1) {
            printf("-------- No devices found, try again\n");
            continue;
        }

        int size = sizeof(inquiry_info) * ir->num_rsp;
        ii = malloc(size);
        if (!ii) {
            fprintf(stderr, "Failed to alloc memory for inquiry info: %s\n", strerror(errno));
            /* free(buf); */
            /* return -1; */
            continue;
        }

        memcpy((void*)ii, buf + sizeof(*ir), size);
        int len = ir->num_rsp;
        /* free(buf); */

        int i = 0;
        char addr[19] = {0};
        for (; i < len; i++) {
            memset(addr, 0, sizeof(addr));
            ba2str(&(ii+i)->bdaddr, addr);
            printf("Remote device address: %s\n", addr);
            printf("\tpscan_rep_mode: %u\n", (ii+i)->pscan_rep_mode);
            printf("\tpscan_period_mode: %u\n", (ii+i)->pscan_period_mode);
            printf("\tpscan_mode: %u\n", (ii+i)->pscan_mode);
            printf("\tdev class: %u, %u, %u\n", (ii+i)->dev_class[0],
                   (ii+i)->dev_class[1],(ii+i)->dev_class[2]);
            printf("\tclock offset: %u\n", (ii+i)->clock_offset);
            printf("\n");
        }
        printf("\n");
        free(ii);
        break;
    }

    free(buf);
    return 0;
}

int
main(int argc, char *argv[])
{
    struct hci_dev_list_req *dl = NULL;
    struct hci_dev_req *dr = NULL;
    int i = 0, ret = 0;

    // connect bluetooth socket
    int sk = socket(AF_BLUETOOTH, SOCK_RAW|SOCK_CLOEXEC, BTPROTO_HCI);
    if (sk < 0) {
        fprintf(stderr, "Failed to connect bluetooth socket: %s\n", strerror(errno));
        return -1;
    }

    // 分配空间
    dl = (struct hci_dev_list_req*)malloc(HCI_MAX_DEV * sizeof(*dr) + sizeof(*dl));
    if (!dl) {
        fprintf(stderr, "Failed to alloc memory for dev list req: %s\n", strerror(errno));
        close(sk);
        return -1;
    }

    memset(dl, 0, HCI_MAX_DEV * sizeof(*dr) + sizeof(*dl));
    dl->dev_num = HCI_MAX_DEV;
    dr = dl->dev_req;

    // 获取本地设备列表
    ret = ioctl(sk, HCIGETDEVLIST, (void*)dl);
    if (ret < 0) {
        fprintf(stderr, "Failed to get dev list: %s\n", strerror(errno));
        close(sk);
        free(dl);
        return -1;
    }

    // 遍历设备
    printf("Device number: %d\n", dl->dev_num);
    for (i = 0; i < dl->dev_num; i++, dr++) {
        ret = query_dev_info(sk, dr->dev_id);
        if (ret != 0) {
            continue;
        }

        scan_nearly_dev(sk, dr->dev_id);
    }

    free(dl);
    close(sk);
    return 0;
}
