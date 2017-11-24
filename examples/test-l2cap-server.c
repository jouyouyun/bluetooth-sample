#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>

#include <pthread.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/l2cap.h>


void * Read_thread(void* pSK);

int main(int argc, char** argv)
{
    int iRel = 0;
    int sk = 0;
    struct sockaddr_l2 local_addr;
    struct sockaddr_l2 remote_addr;
    unsigned int len;
    int nsk = 0;
    pthread_t nth = 0;
    struct l2cap_options opts;
    unsigned int optlen = 0;
    char str[16] = {0};

    if(argc < 2)
    {
        printf("\nUsage:%s psm\n", argv[0]);
        exit(0);
    }

    // create l2cap socket
    sk = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);  //发送数据，使用SOCK_SEQPACKET为好
    if(sk < 0)
    {
        perror("\nsocket():");
        exit(0);
    }


    //bind
    local_addr.l2_family = PF_BLUETOOTH;
    local_addr.l2_psm = htobs(atoi(argv[argc -1]));  //last psm
    bacpy(&local_addr.l2_bdaddr, BDADDR_ANY);
    iRel = bind(sk, (struct sockaddr *)&local_addr, sizeof(struct sockaddr));
    if(iRel < 0)
    {
        perror("\nbind()");
        exit(0);
    }


    //get opts
    // in mtu 和 out mtu.每个包的最大值
    memset(&opts, 0, sizeof(opts));
    optlen = sizeof(opts);
    getsockopt(sk, SOL_L2CAP, L2CAP_OPTIONS, &opts, &optlen);
    printf("\nomtu:[%d]. imtu:[%d]. flush_to:[%d]. mode:[%d]\n", opts.omtu, opts.imtu, opts.flush_to, opts.mode);


    //set opts. default value
    opts.omtu = 0;
    opts.imtu = 672;
    if (setsockopt(sk, SOL_L2CAP, L2CAP_OPTIONS, &opts, sizeof(opts)) < 0)
    {
        perror("\nsetsockopt():");
        exit(0);
    }


    //listen
    iRel = listen(sk, 10);
    if(iRel < 0)
    {
        perror("\nlisten()");
        exit(0);
    }

    len = sizeof(struct sockaddr_l2);
    while(1)
    {
        memset(&remote_addr, 0, sizeof(struct sockaddr_l2));
        nsk = accept(sk, (struct sockaddr*)(&remote_addr), &len);
        if(nsk < 0)
        {
            perror("\naccept():");
            continue;
        }
        ba2str(&(remote_addr.l2_bdaddr), str);
        printf("\npeer bdaddr:[%s].\n", str);  //得到peer的信息

        iRel = pthread_create(&nth, NULL, Read_thread, &nsk);
        if(iRel != 0)
        {
            perror("pthread_create():");
            continue;
        }
        pthread_detach(nth);  // 分离之
    }

    return 0;
}


void * Read_thread(void* pSK)
{
    //struct pollfd fds[10];
    struct   pollfd   fds[100];
    char buf[1024] = {0};
    int iRel = 0;
    int exit_val = 0;

    //fds[0].fd = *(int*)pSK;
    //fds[0].events = POLLIN | POLLHUP;

    fds[0].fd   =   (int)(*(int*)pSK);
    fds[0].events   =   POLLIN   |   POLLHUP;

    while(1)
    {
        if(poll(fds, 1, -1) < 0)
        {
            perror("\npoll():");
        }
        if(fds[0].revents & POLLHUP)
        {
            //hang up
            printf("\n[%d] Hang up\n", *(int*)pSK);
            close(*(int*)pSK);
            pthread_exit(&exit_val);

            break;
        }

        if(fds[0].revents & POLLIN)
        {
            memset(buf, 0 , 1024);
            //read data
            iRel = recv(*(int*)pSK, buf, 572, 0);
            printf("\nHandle[%d] Receive [%d] data:[%s]", *(int*)pSK, iRel, buf);
        }

    }

    return 0;
}
