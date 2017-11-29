/**
 * The local device operations.
 **/

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

static struct hci_dev_list_req *list_local_dev_id();
static inquiry_info* do_inquiry(int dev_id, int *dev_num);

static int
open_bt_socket()
{
    return socket(AF_BLUETOOTH, SOCK_RAW|SOCK_CLOEXEC, BTPROTO_HCI);
}

static void
close_bt_socket(int sk)
{
    if (sk < 0) {
        return ;
    }
    close(sk);
}

struct hci_dev_info*
list_local_dev(int *dev_num)
{
    if (!dev_num) {
        fprintf(stderr, "Invalid dev number argument\n");
        return NULL;
    }

    struct hci_dev_list_req *dl = list_local_dev_id();
    if (!dl) {
        return NULL;
    }

    struct hci_dev_info *infos = NULL;
    struct hci_dev_req *dr = dl->dev_req;
    int i = 0, count = 0;
    for (; i < dl->dev_num; i++, dr++) {
        struct hci_dev_info info;
        int ret = hci_devinfo(dr->dev_id, &info);
        if (ret < 0) {
            fprintf(stderr, "Failed to get dev(%d) info: %s\n",
                    dr->dev_id, strerror(errno));
            continue;
        }

        struct hci_dev_info *tmp = realloc(infos, sizeof(info) * (count+1));
        if (!tmp) {
            fprintf(stderr, "Failed to allocate memory for dev(%d -%s): %s\n",
                    info.dev_id, info.name, strerror(errno));
            continue;
        }

        infos = tmp;
        tmp = NULL;
        memset(infos+count, 0, sizeof(info));
        memcpy(infos+count, &info, sizeof(info));
        count++;
    }

    free(dl);
    *dev_num = count;

    return infos;
}

static inquiry_info*
do_inquiry(int dev_id, int *dev_num)
{
    if (!dev_num) {
        fprintf(stderr, "Invalid dev number argument\n");
        return NULL;
    }

    inquiry_info *ii = NULL;
    int len = 8;
    int max_rsp = 255;
    long flags = IREQ_CACHE_FLUSH;

    ii = malloc(max_rsp * sizeof(*ii));
    if (!ii) {
        fprintf(stderr, "Failed to allocate memory for inquiry info: %s\n",
                strerror(errno));
        return NULL;
    }

    int num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
    if (num_rsp < 0) {
        fprintf(stderr, "Failed to inquiry for dev(%d): %s\n",
                dev_id, strerror(errno));
        free(ii);
        return NULL;
    }

    *dev_num = num_rsp;
    return ii;
}

static struct hci_dev_list_req*
list_local_dev_id()
{
    int sk = open_bt_socket();
    if (sk < 0) {
        fprintf(stderr, "Failed to open bt socket: %s\n", strerror(errno));
        return NULL;
    }

    struct hci_dev_list_req *dl = NULL;

    size_t size = HCI_MAX_DEV * sizeof(struct hci_dev_req) * sizeof(*dl);
    dl = (struct hci_dev_list_req*) malloc(size);
    if (!dl) {
        fprintf(stderr, "Failed to allocate memory for dev list req: %s\n", strerror(errno));
        close_bt_socket(sk);
        return NULL;
    }

    memset(dl, 0, size);
    dl->dev_num = HCI_MAX_DEV;

    int ret = ioctl(sk, HCIGETDEVLIST, (void*)dl);
    if (ret < 0) {
        fprintf(stderr, "Failed to get dev id list: %s\n", strerror(errno));
        free(dl);
        close_bt_socket(sk);
        return NULL;
    }

    return dl;
}
