#include "monkey.h"

/*
use rand & 8191 instead of rand % 10000 to optimize performance.
so the rate argument need to div 1.2207(10000/8192)

TODO: Add 4-state Markov-Model for FEC related testing
*/
static bool event_flag(uint64_t rate)
{
    return (rte_rand() & 8191) < rate;
}

//return CPU tick time
static uint64_t us_to_tsc(uint64_t time_druation_in_us)
{
    return (rte_get_tsc_hz() + US_PER_S - 1) / US_PER_S * time_druation_in_us;
}

struct monkey_metadata {
        uint16_t num; //packet number with the same timestamp
        uint64_t timestamp;
} __rte_cache_aligned;


static inline struct monkey_metadata *
get_priv(struct rte_mbuf *m)
{
        return rte_mbuf_to_priv(m);
}



int lcore_monkey(struct monkey_params *p)
{
    printf("Monkey %u doing packet chaos.\n", rte_lcore_id());

    //allocate ring buffer for delayed packets
    struct rte_ring *delay_ring[2];
    struct rte_ether_hdr *eth_hdr;

    char ring_name[18];
    sprintf(ring_name, "left_ring%d", p->queue_id);
    delay_ring[0] = rte_ring_create(ring_name, p->delay_ring_size,
                                    rte_socket_id(), RING_F_SC_DEQ | RING_F_SP_ENQ);
    if (delay_ring[0] == NULL)
        rte_exit(EXIT_FAILURE, "Cannot create left ring\n");

    sprintf(ring_name, "right_ring%d", p->queue_id);
    delay_ring[1] = rte_ring_create(ring_name, p->delay_ring_size,
                                    rte_socket_id(), RING_F_SC_DEQ | RING_F_SP_ENQ);
    if (delay_ring[1] == NULL)
        rte_exit(EXIT_FAILURE, "Cannot create right ring\n");

    //periodic timer
    uint64_t prev_tsc, diff_tsc, cur_tsc, timer_tsc;
    const uint64_t drain_tsc = (rte_get_tsc_hz() + US_PER_S - 1) / US_PER_S *
                               BURST_TX_DRAIN_US;
    const uint64_t adjust_tsc_multiplier = 1 * US_PER_S / BURST_TX_DRAIN_US;
    uint64_t adjust_cnt = 0;
    prev_tsc = 0;
    timer_tsc = 0;

    //buffers and counters
    struct rte_mbuf *pkts[BURST_SIZE];
    struct rte_mbuf *tx_pkts[2][BURST_SIZE_TX];
    struct rte_eth_dev_tx_buffer *tx_buffer[2];
    for (int i = 0; i < 2; i++)
    {
        tx_buffer[i] = rte_zmalloc_socket("tx_buffer",
                                          RTE_ETH_TX_BUFFER_SIZE(BURST_SIZE * 2),
                                          0, rte_eth_dev_socket_id(i));
        if (tx_buffer[i] == NULL)
            rte_exit(EXIT_FAILURE, "Cannot allocate buffer for Core: %d on port[%d]\n", p->queue_id, i);

        int retval = rte_eth_tx_buffer_init(tx_buffer[i], BURST_SIZE * 2);
        if (retval < 0)
            rte_exit(EXIT_FAILURE, "Cannot allocate buffer for Core: %d on port[%d]\n", p->queue_id, i);
    }

    int tx_head[2] = {0, 0};
    int tx_tail[2] = {0, 0};
    uint64_t latency_tsc[2] = {0, 0};
    uint64_t loss_rate[2] = {0, 0};
    uint64_t dup_rate[2] = {0, 0};
    uint64_t latency[2]= {0,0};
    uint64_t jitter[2] = {0,0};

    //stats
    uint64_t rx_pkt_cnt[2] = {0, 0};
    uint64_t drop_pkt_cnt[2] = {0, 0};
    uint64_t failed_enq_cnt[2] = {0, 0};
    uint64_t deq_pkt_cnt[2] = {0, 0};
    uint64_t tx_pkt_cnt[2] = {0, 0};
    uint64_t tx_bytes_cnt[2]= {0,0};

    
    while (!force_quit)
    {
        cur_tsc = rte_rdtsc();
        //check system timer and update every adjust_tsc_multiplier x us.
        if (unlikely(adjust_cnt == adjust_tsc_multiplier))
        {

            for (int i = 0; i < 2; i++)
            {
                rte_atomic64_set(&p->stats[i]->tx_pkt_cnt, tx_pkt_cnt[i] );
                rte_atomic64_set(&p->stats[i]->tx_bytes, tx_bytes_cnt[i] );
                rte_atomic64_set(&p->stats[i]->rx_pkt_cnt, rx_pkt_cnt[i] );
                rte_atomic64_set(&p->stats[i]->drop_pkt_cnt, drop_pkt_cnt[i]);
                rte_atomic64_set(&p->stats[i]->deq_pkt_cnt, deq_pkt_cnt[i]);
                rte_atomic64_set(&p->stats[i]->failed_enq_cnt, failed_enq_cnt[i] );
                rte_atomic64_set(&p->stats[i]->q_depth, rte_ring_count(delay_ring[i]));
                
                loss_rate[i] = rte_atomic64_read(&p->config[i]->drop_rate) / 1.22;
                
                jitter[i] = rte_atomic64_read(&p->config[i]->jitter);
                latency[i] = rte_atomic64_read(&p->config[i]->latency)- jitter[i];
                jitter[i] = jitter[i]*2;
                latency_tsc[i] = us_to_tsc(latency[i] + rte_rand_max(jitter[i]));

                rx_pkt_cnt[i] = 0;
                tx_pkt_cnt[i] = 0;
                drop_pkt_cnt[i] = 0;
                failed_enq_cnt[i] = 0;
                deq_pkt_cnt[i] = 0;
                tx_bytes_cnt[i]=0;
            }
            adjust_cnt = 0;
        }

        diff_tsc = cur_tsc - prev_tsc;
        if (unlikely(diff_tsc > drain_tsc))
        {
            for (int i = 0; i < 2; i++)
            {
                rte_eth_tx_buffer_flush(i, p->queue_id, tx_buffer[i]);
            }
            prev_tsc = cur_tsc;
            adjust_cnt++;
        }

       //Packet RX with Loss 
        for (uint16_t port_id = 0; port_id < 2; port_id++)
        {

            uint16_t egress_id = (port_id + 1) & 1;
            struct rte_ring *ring = delay_ring[egress_id];
            uint64_t loss = loss_rate[port_id];

            const uint16_t nb_rx = rte_eth_rx_burst(port_id, p->queue_id, pkts, BURST_SIZE);
            if (unlikely(nb_rx == 0))
            {
                continue;
            }
            rx_pkt_cnt[port_id] += nb_rx;
            uint16_t loss_cnt = 0;
            for (int i = 0; i < nb_rx; i++)
            {
                if (event_flag(loss))
                {
                    loss_cnt++;
                }
            }
            
            //Optimization: assume nb_rx packets have the same timestamp, just write ts on first pkt.
            struct monkey_metadata *mdata = get_priv(pkts[0]);
            mdata->num = nb_rx-loss;
            mdata->timestamp = cur_tsc;   

            uint16_t sent = rte_ring_enqueue_burst(ring, (void *)pkts, nb_rx - loss_cnt, NULL);
            drop_pkt_cnt[egress_id] += (nb_rx - sent);
            if (unlikely(sent < nb_rx))
            {
                failed_enq_cnt[egress_id] += (nb_rx - loss_cnt - sent);
                while (sent < nb_rx)
                    rte_pktmbuf_free(pkts[sent++]);
            }
        }


        // Packet TX with Latency and Duplicate
        //TODO: Duplicate could based on mbuf_clone during send.
        //TODO: ReOrder could based on random send in tx_queue.
        for (uint16_t ring_id = 0; ring_id < 2; ring_id++)
        {
            cur_tsc = rte_rdtsc();
            if (unlikely(tx_head[ring_id] == tx_tail[ring_id]))
            {
                //rte_pktmbuf_free_bulk(tx_pkts[ring_id], tx_tail[ring_id]);
                const uint16_t nb_rx = rte_ring_dequeue_burst(delay_ring[ring_id],
                                                              (void *)tx_pkts[ring_id], BURST_SIZE_TX, NULL);
                tx_head[ring_id] = 0;
                tx_tail[ring_id] = nb_rx;
                deq_pkt_cnt[ring_id] += nb_rx;
            }
            uint16_t egress_id = (ring_id + 1) & 1;

            // Prefetch first packets
            int i;
            for (i = tx_head[ring_id]; i < PREFETCH_OFFSET && i < tx_tail[ring_id]; i++)
            {
                rte_prefetch0(rte_pktmbuf_mtod(tx_pkts[ring_id][i], void *));
            }

            for (i = tx_head[ring_id]; i < (tx_tail[ring_id] - PREFETCH_OFFSET); i++)
            {
                rte_prefetch0(rte_pktmbuf_mtod(tx_pkts[ring_id][i + PREFETCH_OFFSET], void *));

                struct monkey_metadata *mdata = get_priv(tx_pkts[ring_id][i]);
                if (unlikely( ( mdata->timestamp > 0 ) && (cur_tsc - mdata->timestamp < latency_tsc[egress_id]) )) {
                    i = tx_tail[ring_id];
                    break;
                } 
                if (p->enable_mac_update > 0) {
                   eth_hdr = rte_pktmbuf_mtod(tx_pkts[ring_id][i], struct rte_ether_hdr *);
                   rte_ether_addr_copy(&p->src_mac[ring_id],&eth_hdr->s_addr);
                   rte_ether_addr_copy(&p->dst_mac[ring_id],&eth_hdr->d_addr);
                }
                rte_eth_tx_buffer(ring_id, p->queue_id, tx_buffer[ring_id], tx_pkts[ring_id][i]);
                tx_pkt_cnt[ring_id]++;
                tx_bytes_cnt[ring_id]+= rte_pktmbuf_pkt_len(tx_pkts[ring_id][i]);
                tx_head[ring_id]++;
            }
            // Process left packets
            for (; i < tx_tail[ring_id]; i++)
            {
                struct monkey_metadata *mdata = get_priv(tx_pkts[ring_id][i]);
                if (unlikely( ( mdata->timestamp > 0 ) && (cur_tsc - mdata->timestamp < latency_tsc[egress_id]) )) {
                    i = tx_tail[ring_id];
                    break;
                } 
                if (p->enable_mac_update > 0) {
                   eth_hdr = rte_pktmbuf_mtod(tx_pkts[ring_id][i], struct rte_ether_hdr *);
                   rte_ether_addr_copy(&p->src_mac[ring_id],&eth_hdr->s_addr);
                   rte_ether_addr_copy(&p->dst_mac[ring_id],&eth_hdr->d_addr);
                }
                rte_eth_tx_buffer(ring_id, p->queue_id, tx_buffer[ring_id], tx_pkts[ring_id][i]);
                tx_pkt_cnt[ring_id]++;
                tx_bytes_cnt[ring_id]+= rte_pktmbuf_pkt_len(tx_pkts[ring_id][i]);
                tx_head[ring_id]++;
            }
        }
    }
    return 0;
}
