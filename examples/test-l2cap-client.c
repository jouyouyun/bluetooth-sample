#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>


#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/l2cap.h>


int main(int argc, char** argv)
{
    int sk;
    int i = 0;
    char buf[24] = "Sam is Good Guy!";
    struct sockaddr_l2 local_addr;
    struct sockaddr_l2 remote_addr;
    int iRel = 0;

    if(argc < 3)
    {
        printf("\nUsage:%s <bdaddr> <PSM>\n", argv[0]);
        exit(0);
    }


    sk = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
    if(sk < 0)
    {
        perror("\nsocket():");
        exit(0);
    }

    //bind. bluetooth好像不许有无名Socket
    local_addr.l2_family = PF_BLUETOOTH;
    bacpy(&local_addr.l2_bdaddr, BDADDR_ANY);
    iRel = bind(sk, (struct sockaddr *)&local_addr, sizeof(struct sockaddr));
    if(iRel < 0)
    {
        perror("\nbind()");
        exit(0);
    }


    memset(&remote_addr, 0, sizeof(struct sockaddr_l2));
    remote_addr.l2_family = PF_BLUETOOTH;
    str2ba(argv[1], &remote_addr.l2_bdaddr);
    remote_addr.l2_psm = htobs(atoi(argv[argc -1]));

    connect(sk, (struct sockaddr*)&remote_addr, sizeof(struct sockaddr_l2));

    for(i = 0; i < 60; i++)
    {
        iRel = send(sk, buf, strlen(buf)+1, 0);
        printf("Send [%ld] data\n", strlen(buf)+1);
        sleep(1);
    }

    close(sk);
    return 0;
}
