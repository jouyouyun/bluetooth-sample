/**
 * The local device oprations
 **/

#ifndef __BT_LOCAL_H__
#define __BT_LOCAL_H__

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

typedef struct _remote_dev_info {
    inquiry_info ii;
    char name[248];
} remote_dev_info;

struct hci_dev_info *list_local_dev(int *dev_num);
struct remote_dev_info *list_nearly_dev(int dev_id, int *dev_num);

#endif
