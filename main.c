#include <stdint.h>
#include <unistd.h>
#include <inttypes.h>

#include <pthread.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>

#include "portinit.h"
#include "monkey.h"
#include "cli_parser.h"

int main(int argc, char *argv[])
{

    struct rte_mempool *mbuf_pool[MAX_SERVICE_CORE];

    config.num_service_core = 1;
    config.first_lcore = 1;
    config.ctrl_udp_port = 6666;
    config.delay_ring_size = 1024 * 1024; // each of ring has 1M elements to handle delayed pkts
    config.mbuf_size = 65536;             // for 100G must be 2*1024 * 1024, config with --mbuf_size;
    config.max_linkspeed = 0;
    // left->right
    config.latency[0] = 0;   // microsecond
    config.jitter[0] = 0;    // microsecond
    config.drop_rate[0] = 0; // 1/10000
    config.dup_rate[0] = 0;  // 1/10000

    // right->left
    config.latency[1] = 0;   // microsecond
    config.jitter[1] = 0;    // microsecond
    config.drop_rate[1] = 0; // 1/10000
    config.dup_rate[1] = 0;  // 1/10000

    int retval = rte_eal_init(argc, argv);
    if (retval < 0)
        rte_exit(EXIT_FAILURE, "Initialize fail!");

    force_quit = false;
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Check MAX memory segments
    printf("RTE_MAX_HEAPS: %8d\n", RTE_MAX_HEAPS);
    printf("RTE_MAX_MEMSEG_LISTS: %8d\n", RTE_MAX_MEMSEG_LISTS);
    printf("RTE_MAX_MEMSEG_PER_LIST: %8d\n", RTE_MAX_MEMSEG_PER_LIST);
    printf("RTE_MAX_MEM_MB_PER_LIST: %8d\n", RTE_MAX_MEM_MB_PER_LIST);
    printf("RTE_MAX_MEMSEG_PER_TYPE: %8d\n", RTE_MAX_MEMSEG_PER_TYPE);
    printf("RTE_MAX_MEM_MB_PER_TYPE: %8d\n", RTE_MAX_MEM_MB_PER_TYPE);

    struct rte_eth_link link;
    retval = rte_eth_link_get_nowait(LEFT_PORT, &link);
    config.max_linkspeed = link.link_speed;
    retval = rte_eth_link_get_nowait(RIGHT_PORT, &link);
    if (link.link_speed > config.max_linkspeed)
    {
        config.max_linkspeed = link.link_speed;
    }
    printf("max linkspeed is : %d Gbps\n", config.max_linkspeed / 1000);

    // TODO based on linkspeed allocate memory ?

    // TODO: Add CLI parser or directly use remote client.
    retval = zmonkey_args_parser(argc, argv, &config);
    if (retval < 0)
        rte_exit(EXIT_FAILURE, "Invalid arguments\n");

    printf("num_service_core: %d\n", config.num_service_core);
    printf("num_mb_elements: %d\n", config.mbuf_size);
    printf("delay ring size per core: %d\n", config.delay_ring_size);
    printf("control udp port: %d\n", config.ctrl_udp_port);

    if (config.enable_mac_update > 0)
    {

        print_ether_addr("Left Port", &config.src_mac[0], &config.dst_mac[0]);
        print_ether_addr("Right Port", &config.src_mac[1], &config.dst_mac[1]);
    }

    /* Creates a new mempool in memory to hold the mbufs. */
    for (int i = 0; i < config.num_service_core; ++i)
    {
        char mbuf_pool_name[18];
        sprintf(mbuf_pool_name, "MBUF_POOL%d", i);
        mbuf_pool[i] = rte_pktmbuf_pool_create(mbuf_pool_name, config.mbuf_size,
                                               MBUF_CACHE_SIZE, 8,
                                               RTE_MBUF_DEFAULT_BUF_SIZE, rte_lcore_to_socket_id(config.first_lcore + i));
        if (mbuf_pool[i] == NULL)
            rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");
        printf("%s allocated.\n", mbuf_pool_name);
    }

    /* Initialize eth port */
    if (port_init(LEFT_PORT, mbuf_pool, config.num_service_core, config.num_service_core) != 0)
        rte_exit(EXIT_FAILURE, "Cannot init port %" PRIu16 "\n", LEFT_PORT);
    if (port_init(RIGHT_PORT, mbuf_pool, config.num_service_core, config.num_service_core) != 0)
        rte_exit(EXIT_FAILURE, "Cannot init port %" PRIu16 "\n", RIGHT_PORT);

    printf("Starting service process...\n");

    /* Start Monkey process */
    unsigned int lcore_num = config.first_lcore;
    struct monkey_params *lp = (struct monkey_params *)malloc(sizeof(struct monkey_params) * config.num_service_core);

    for (int i = 0; i < config.num_service_core; ++i)
    {
        lp[i].queue_id = i;
        lp[i].delay_ring_size = config.delay_ring_size;

        for (int j = 0; j < 2; j++)
        {
            lp[i].config[j] = malloc(sizeof(struct chaos_config));
            lp[i].stats[j] = malloc(sizeof(struct stats));

            rte_atomic64_set(&lp[i].config[j]->latency, config.latency[j]);
            rte_atomic64_set(&lp[i].config[j]->jitter, config.jitter[j]);
            rte_atomic64_set(&lp[i].config[j]->drop_rate, config.drop_rate[j]);
            rte_atomic64_set(&lp[i].config[j]->dup_rate, config.dup_rate[j]);

            rte_atomic64_set(&lp[i].stats[j]->tx_pkt_cnt, 0);
            rte_atomic64_set(&lp[i].stats[j]->rx_pkt_cnt, 0);
            rte_atomic64_set(&lp[i].stats[j]->drop_pkt_cnt, 0);
            rte_atomic64_set(&lp[i].stats[j]->deq_pkt_cnt, 0);
            rte_atomic64_set(&lp[i].stats[j]->failed_enq_cnt, 0);
            rte_atomic64_set(&lp[i].stats[j]->q_depth, 0);
        }
        if (config.enable_mac_update > 0)
        {
            rte_ether_addr_copy(&config.src_mac[0], &lp->src_mac[0]);
            rte_ether_addr_copy(&config.src_mac[1], &lp->src_mac[1]);
            rte_ether_addr_copy(&config.dst_mac[0], &lp->dst_mac[0]);
            rte_ether_addr_copy(&config.dst_mac[1], &lp->dst_mac[1]);
        }
        rte_eal_remote_launch((lcore_function_t *)lcore_monkey, &lp[i], lcore_num++);
    }

    // control process
    int sin_len;
    char buff[256];

    int sock_fd;
    struct sockaddr_in sin;

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(config.ctrl_udp_port);
    sin_len = sizeof(sin);

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 900000;

    if (setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
    {
        perror("setsockopt timeout failed.");
        exit(EXIT_FAILURE);
    }
    bind(sock_fd, (struct sockaddr *)&sin, sizeof(sin));

    while (!force_quit)
    {
        int n = recvfrom(sock_fd, buff, sizeof(buff), 0, (struct sockaddr *)&sin, (socklen_t *)&sin_len);
        if (n > 0)
        {
            int instr = -1;
            int direction = -1;
            uint64_t value = -1;

            // buff[n+1] = '\0';
            char *p = strtok(buff, ",");
            if (!p)
                continue;
            instr = atoi(p);
            p = strtok(NULL, ",");
            if (!p)
                continue;
            direction = atoi(p);
            if ((direction > 1) || (direction < 0))
                continue;

            p = strtok(NULL, ",");
            if (!p)
                continue;
            value = strtoull(p, NULL, 10);
            switch (instr)
            {
            case 1:
                // set latency value
                config.latency[direction] = value;
                for (int i = 0; i < config.num_service_core; ++i)
                    rte_atomic64_set(&lp[i].config[direction]->latency, config.latency[direction]);
                break;
            case 2:
                // set jitter value
                config.jitter[direction] = value;
                for (int i = 0; i < config.num_service_core; ++i)
                    rte_atomic64_set(&lp[i].config[direction]->jitter, config.jitter[direction]);
                break;
            case 3:
                // set drop value
                config.drop_rate[direction] = value;
                for (int i = 0; i < config.num_service_core; ++i)
                    rte_atomic64_set(&lp[i].config[direction]->drop_rate, config.drop_rate[direction]);
                break;
            case 4:
                // set dup value
                config.dup_rate[direction] = value;
                for (int i = 0; i < config.num_service_core; ++i)
                    rte_atomic64_set(&lp[i].config[direction]->dup_rate, config.dup_rate[direction]);
                break;
            case 65535:
                printf("Controller send msg to shutdown service\n");
                force_quit = true;
                close(sock_fd);
                exit(0);
                rte_eal_cleanup();
                return (EXIT_SUCCESS);
            default:
                break;
            }
        }
        print_stats(&config, lp, config.num_service_core);
    }
    close(sock_fd);

    return 0;
}
