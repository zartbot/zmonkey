#ifndef __RUTA_COMMON_H_
#define __RUTA_COMMON_H_

#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_malloc.h>
#include <rte_mbuf.h>
#include <rte_hash_crc.h>
#include <rte_bus_vdev.h>
#include <rte_ether.h>
#include <rte_cryptodev.h>
#include <rte_ip.h>
#include <rte_udp.h>
#include <inttypes.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>

#define RX_RING_SIZE 128
#define TX_RING_SIZE 128

#define MBUF_CACHE_SIZE 128

#define PREFETCH_OFFSET 4
#define MAX_SERVICE_CORE 32

#define LEFT_PORT 0
#define RIGHT_PORT 1

#define BURST_SIZE 32
#define BURST_SIZE_TX 32
#define BURST_TX_DRAIN_US 100

struct config
{
    int num_service_core;
    int first_lcore;
    uint16_t ctrl_udp_port;
    unsigned int mbuf_size;
    unsigned int delay_ring_size;
    uint32_t max_linkspeed;

    uint64_t latency[2];
    uint64_t jitter[2];
    uint64_t drop_rate[2];
    uint64_t dup_rate[2];
    int enable_mac_update;
    struct rte_ether_addr src_mac[2];
    struct rte_ether_addr dst_mac[2];
};

static struct config config;



struct chaos_config
{
    rte_atomic64_t latency;
    rte_atomic64_t jitter;
    rte_atomic64_t drop_rate;
    rte_atomic64_t dup_rate;
};

struct stats
{
    rte_atomic64_t rx_pkt_cnt;
    rte_atomic64_t tx_pkt_cnt;
    rte_atomic64_t drop_pkt_cnt;
    rte_atomic64_t deq_pkt_cnt;
    rte_atomic64_t failed_enq_cnt;
    rte_atomic64_t q_depth;
    rte_atomic64_t tx_bytes;
};

struct stats_cnt
{
    uint16_t queue_id;
    uint64_t rx_pkt_cnt[2];
    uint64_t tx_pkt_cnt[2];
    uint64_t drop_pkt_cnt[2];
    uint64_t deq_pkt_cnt[2];
    uint64_t failed_enq_cnt[2];
    uint64_t q_depth[2];
    uint64_t tx_bytes[2];
};

struct monkey_params
{
    struct rte_ring *ring[2];
    struct rte_mempool *mem_pool;
    uint16_t queue_id;
    unsigned int delay_ring_size;
    struct chaos_config *config[2];
    struct stats *stats[2];
    int enable_mac_update;
    struct rte_ether_addr src_mac[2];
    struct rte_ether_addr dst_mac[2];
};

static void read_stats(struct monkey_params *p, struct stats_cnt *s)
{
    s->queue_id = p->queue_id;
    for (int i = 0; i < 2; i++)
    {
        s->rx_pkt_cnt[i] = rte_atomic64_read(&p->stats[i]->rx_pkt_cnt);
        s->tx_pkt_cnt[i] = rte_atomic64_read(&p->stats[i]->tx_pkt_cnt);
        s->drop_pkt_cnt[i] = rte_atomic64_read(&p->stats[i]->drop_pkt_cnt);
        s->deq_pkt_cnt[i] = rte_atomic64_read(&p->stats[i]->deq_pkt_cnt);
        s->failed_enq_cnt[i] = rte_atomic64_read(&p->stats[i]->failed_enq_cnt);
        s->q_depth[i] = rte_atomic64_read(&p->stats[i]->q_depth);
        s->tx_bytes[i] = rte_atomic64_read(&p->stats[i]->tx_bytes);
    }
}

static void z_stats(struct stats_cnt *s)
{
    s->queue_id = 0;
    for (int i = 0; i < 2; i++)
    {
        s->rx_pkt_cnt[i] = 0;
        s->tx_pkt_cnt[i] = 0;
        s->drop_pkt_cnt[i] = 0;
        s->deq_pkt_cnt[i] = 0;
        s->failed_enq_cnt[i] = 0;
        s->q_depth[i] = 0;
        s->tx_bytes[i] = 0;
    }
}

static void sum_stats(struct stats_cnt *d, struct stats_cnt *s)
{
    for (int i = 0; i < 2; i++)
    {
        d->rx_pkt_cnt[i] += s->rx_pkt_cnt[i];
        d->tx_pkt_cnt[i] += s->tx_pkt_cnt[i];
        d->drop_pkt_cnt[i] += s->drop_pkt_cnt[i];
        d->deq_pkt_cnt[i] += s->deq_pkt_cnt[i];
        d->failed_enq_cnt[i] += s->failed_enq_cnt[i];
        d->q_depth[i] += s->q_depth[i];
        d->tx_bytes[i] += s->tx_bytes[i];
    }
}

static int
print_stats_l2r(struct stats_cnt *s)
{
    printf("Monkey[%3d] %9ld --> LeftPort --> [Drop %9ld -->(Q-Depth %9ld|Fail %9ld)--> DeQ  %9ld] --> RightPort --> %9ld\n",
           s->queue_id,
           s->rx_pkt_cnt[0],
           s->drop_pkt_cnt[1],
           s->q_depth[1],
           s->failed_enq_cnt[1],
           s->deq_pkt_cnt[1],
           s->tx_pkt_cnt[1]);
}

static int
print_stats_r2l(struct stats_cnt *s)
{
    printf("Monkey[%3d] %9ld <-- LeftPort <-- [DeQ  %9ld <--(Q-Depth %9ld|Fail %9ld)<-- Drop %9ld] <-- RightPort <-- %9ld\n",
           s->queue_id,
           s->tx_pkt_cnt[0],
           s->deq_pkt_cnt[0],
           s->q_depth[0],
           s->failed_enq_cnt[0],
           s->drop_pkt_cnt[0],
           s->rx_pkt_cnt[1]);
}

static void clean_screen()
{
    printf("\e[1;1H\e[2J");
}

static void print_line(char *p)
{
    for (int i = 0; i < 140; i++)
    {
        printf("%s", p);
    }
    printf("\n");
}

static void
print_stats(struct config *config, struct monkey_params *lp, int num_service_core)
{
    struct stats_cnt total;
    struct stats_cnt stats[num_service_core];
    z_stats(&total);

    for (int i = 0; i < num_service_core; i++)
    {
        read_stats(&lp[i], &stats[i]);
        sum_stats(&total, &stats[i]);
    }
    clean_screen();
    print_line("=");
    printf("\n zMonkey Network Chaos System (by zartbot)\n");
    printf("\n\n    LeftPort-------->>>>>>>>>>--------->>>>>>>>>          ");
    printf("    |              <<<<<<<<<--------<<<<<<<<<<---------RightPort\n");
    printf("                                                              |\n");
    printf("        Latency   :%10ld us", config->latency[0]);
    printf("                              |        ");
    printf("        Latency   :%10ld us\n", config->latency[1]);
    printf("        Jitter    :%10ld us", config->jitter[0]);
    printf("                              |        ");
    printf("        Jitter    :%10ld us\n", config->jitter[1]);

    printf("        Loss      :%10ld %%%%", config->drop_rate[0]);
    printf("                              |        ");
    printf("        Loss      :%10ld %%%%\n", config->drop_rate[1]);
    printf("        Duplicate :%10ld %%%%", config->dup_rate[0]);
    printf("                              |        ");
    printf("        Duplicate :%10ld %%%%\n", config->dup_rate[1]);
    printf("                                                              |\n");

    printf("        Total RX  :%10ld pps  --->", total.rx_pkt_cnt[0]);
    printf("                       |        ");
    printf("        <--- Total RX  :%10ld pps \n", total.rx_pkt_cnt[1]);

    printf("        Total TX  :%10ld pps  <---", total.tx_pkt_cnt[0]);
    printf("                       |        ");
    printf("        ---> Total TX  :%10ld pps \n", total.tx_pkt_cnt[1]);
    printf("                                                              |\n");
    printf("        Total TX  :%10ld Mbps <---", total.tx_bytes[0]/131072); //Bytes * 8/1024/1024 to Mbps
    printf("                       |        ");
    printf("        ---> Total TX  :%10ld Mbps \n\n", total.tx_bytes[1]/131072);
    
    print_line("-");
    for (int i = 0; i < num_service_core; i++)
    {
        print_stats_l2r(&stats[i]);
    }
    print_line("-");
    for (int i = 0; i < num_service_core; i++)
    {

        print_stats_r2l(&stats[i]);
    }
    print_line("=");
}

static void print_ether_addr(char *prefix,struct rte_ether_addr *src,struct rte_ether_addr *dst){
         printf("%s %02X:%02X:%02X:%02X:%02X:%02X -> %02X:%02X:%02X:%02X:%02X:%02X \n",
               prefix,
               src->addr_bytes[0], src->addr_bytes[1],
               src->addr_bytes[2], src->addr_bytes[3],
               src->addr_bytes[4], src->addr_bytes[5],
               dst->addr_bytes[0], dst->addr_bytes[1],
               dst->addr_bytes[2], dst->addr_bytes[3],
               dst->addr_bytes[4], dst->addr_bytes[5]);
}

static volatile bool force_quit;
static void
signal_handler(int s)
{
    if (s == SIGINT || s == SIGTERM)
    {
        printf("Signal %d received, preparing to exit...\n", s);
        force_quit = true;
    }
}

#define NIPQUAD(addr)                \
    ((unsigned char *)&addr)[0],     \
        ((unsigned char *)&addr)[1], \
        ((unsigned char *)&addr)[2], \
        ((unsigned char *)&addr)[3]

static inline int
print_pkt(struct rte_mbuf *pkt)
{

    struct rte_ether_hdr *eth_hdr;
    struct rte_ipv4_hdr *ipv4_hdr;
    struct rte_udp_hdr *udp_hdr;
    struct rte_tcp_hdr *tcp_hdr;

    eth_hdr = rte_pktmbuf_mtod(pkt, struct rte_ether_hdr *);
    printf("%02X:%02X:%02X:%02X:%02X:%02X -> %02X:%02X:%02X:%02X:%02X:%02X \n",

           eth_hdr->s_addr.addr_bytes[0], eth_hdr->s_addr.addr_bytes[1],
           eth_hdr->s_addr.addr_bytes[2], eth_hdr->s_addr.addr_bytes[3],
           eth_hdr->s_addr.addr_bytes[4], eth_hdr->s_addr.addr_bytes[5],
           eth_hdr->d_addr.addr_bytes[0], eth_hdr->d_addr.addr_bytes[1],
           eth_hdr->d_addr.addr_bytes[2], eth_hdr->d_addr.addr_bytes[3],
           eth_hdr->d_addr.addr_bytes[4], eth_hdr->d_addr.addr_bytes[5]);

    if (pkt->packet_type & RTE_PTYPE_L3_IPV4)
    {

        uint32_t l4 = pkt->packet_type & RTE_PTYPE_L4_MASK;

        ipv4_hdr = rte_pktmbuf_mtod_offset(pkt, struct rte_ipv4_hdr *, sizeof(struct rte_ether_hdr));
        if (l4 == RTE_PTYPE_L4_ICMP)
        {
            printf("ICMP: %d.%d.%d.%d->%d.%d.%d.%d\n",
                   NIPQUAD(ipv4_hdr->src_addr),
                   NIPQUAD(ipv4_hdr->dst_addr));
        }

        if (l4 == RTE_PTYPE_L4_UDP)
        {
            udp_hdr = rte_pktmbuf_mtod_offset(pkt, struct rte_udp_hdr *, sizeof(struct rte_ether_hdr) + sizeof(struct rte_ipv4_hdr));
            printf("UDP: %d.%d.%d.%d:%d->%d.%d.%d.%d:%d\n",
                   NIPQUAD(ipv4_hdr->src_addr), rte_be_to_cpu_16(udp_hdr->src_port),
                   NIPQUAD(ipv4_hdr->dst_addr), rte_be_to_cpu_16(udp_hdr->dst_port));
        }

        if (l4 == RTE_PTYPE_L4_TCP)
        {
            tcp_hdr = rte_pktmbuf_mtod_offset(pkt, struct rte_tcp_hdr *, sizeof(struct rte_ether_hdr) + sizeof(struct rte_ipv4_hdr));
            printf("TCP: %d.%d.%d.%d:%d->%d.%d.%d.%d:%d\n",
                   NIPQUAD(ipv4_hdr->src_addr), rte_be_to_cpu_16(tcp_hdr->src_port),
                   NIPQUAD(ipv4_hdr->dst_addr), rte_be_to_cpu_16(tcp_hdr->dst_port));
        }
    }
}

static inline void
parse_eth_ptype(struct rte_mbuf *m)
{
    struct rte_ether_hdr *eth_hdr;
    uint32_t packet_type = RTE_PTYPE_UNKNOWN;
    uint16_t ether_type;
    eth_hdr = rte_pktmbuf_mtod(m, struct rte_ether_hdr *);
    ether_type = eth_hdr->ether_type;

    if (ether_type == rte_cpu_to_be_16(RTE_ETHER_TYPE_IPV4))
    {
        packet_type |= RTE_PTYPE_L3_IPV4;
    }
    else if (ether_type == rte_cpu_to_be_16(RTE_ETHER_TYPE_IPV6))
    {
        packet_type |= RTE_PTYPE_L3_IPV6;
    }
    else if (ether_type == rte_cpu_to_be_16(RTE_ETHER_TYPE_ARP))
    {
        packet_type |= RTE_PTYPE_L2_ETHER_ARP;
    }
    m->packet_type = packet_type;
}

static inline void
parse_ipv4_ptype(struct rte_mbuf *m)
{
    struct rte_ipv4_hdr *ipv4_hdr;
    uint32_t packet_type = m->packet_type;

    ipv4_hdr = rte_pktmbuf_mtod_offset(m, struct rte_ipv4_hdr *, sizeof(struct rte_ether_hdr));
    if (ipv4_hdr->next_proto_id == IPPROTO_UDP)
    {
        packet_type |= RTE_PTYPE_L4_UDP;
    }
    else if (ipv4_hdr->next_proto_id == IPPROTO_TCP)
    {
        packet_type |= RTE_PTYPE_L4_TCP;
    }
    else if (ipv4_hdr->next_proto_id == IPPROTO_ICMP)
    {
        packet_type |= RTE_PTYPE_L4_ICMP;
    }
    m->packet_type = packet_type;
}

#endif /* __RUTA_COMMON_H_ */
