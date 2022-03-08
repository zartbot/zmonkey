#include "cli_parser.h"
#include <getopt.h>

int powerOfTwo(int n)
{
    return n && (!(n & (n - 1)));
}

static int64_t
parse_int(const char *arg)
{
    char *end = NULL;
    int64_t result = strtoll(arg, &end, 10);
    if ((arg[0] == '\0') || (end == NULL) || (*end != '\0'))
        return -1;
    return result;
}

/*
 * helper function for parse_mac, parse one section of the ether addr.
 */
static const char *
parse_uint8x16(const char *s, uint8_t *v, uint8_t ls)
{
    char *end;
    unsigned long t;
    errno = 0;
    t = strtoul(s, &end, 16);
    if (errno != 0 || end[0] != ls || t > UINT8_MAX)
        return NULL;
    v[0] = t;
    return end + 1;
}
static int
parse_mac(const char *str, struct rte_ether_addr *addr)
{
    uint32_t i;
    static const uint8_t stop_sym[RTE_DIM(addr->addr_bytes)] = {
        [0] = ':',
        [1] = ':',
        [2] = ':',
        [3] = ':',
        [4] = ':',
        [5] = 0,
    };
    for (i = 0; i != RTE_DIM(addr->addr_bytes); i++)
    {
        str = parse_uint8x16(str, addr->addr_bytes + i, stop_sym[i]);
        if (str == NULL)
            return -EINVAL;
    }
    return 0;
}

void zmonkey_usage()
{
    printf("\n\nzmonkey [EAL options] -- <Parameters>\n\n");
    printf(" -f --first_lcore         First lcore used for forwarding thread\n"
           " -n --core_num            Number of lcore used for forwarding\n"
           " -m --mbuf_size           Number of elements in mbuf\n"
           " -r --ring_size           Number of elements in mbuf\n"
           " -c --control_port        Remote control udp port(default:6666)\n"
           "\nzMonkey chaos config\n\n"
           " -d --l2r_latency         Left  -> Right Delay time [us]\n"
           " -D --r2l_latency         Right -> Left Delay time [us]\n"
           " -j --l2r_jitter          Left  -> Right Jitter time [us]\n"
           " -J --r2l_jitter          Right -> Left Jitter time [us]\n"
           " -l --l2r_loss            Left  -> Right Loss rate [%%%%]\n"
           " -L --r2l_loss            Right -> Left Loss rate [%%%%]\n"
           " -u --l2r_dup             Left  -> Right Duplicate rate [%%%%]\n"
           " -U --r2l_dup             Right -> Left Duplicate rate [%%%%]\n\n\n"
           " -y --r2l_src_mac         Right -> Left Source      MAC[xx:xx:xx:xx:xx:xx]\n"
           " -Y --r2l_dst_mac         Right -> Left Destination MAC[xx:xx:xx:xx:xx:xx]\n"
           " -z --l2r_src_mac         Left  -> Right Source      MAC[xx:xx:xx:xx:xx:xx]\n"
           " -Z --l2r_dst_mac         Left  -> Right Destination MAC[xx:xx:xx:xx:xx:xx]\n\n\n"
           "Example:\n\n"
           "8-Thread to handle 100G/100ms latency with 12.34%% packet drop_rate\n\n"
           "     zmonkey -- --first_lcore 24 --core_num 8  --mbuf_size 2097152 --l2r_latency 100000 --l2r_loss 1234\n\n"
           "16-Thread with 1M element buffer per thread,first lcore start at core.24\n\n"
           "     zmonkey -- --first_lcore 24 --core_num 16  --mbuf_size 1048576 --l2r_latency 10000 \n\n"
           "Short parameters\n\n"
           "     zmonkey -- -f 24 -n 12 -m 2097152 -d 100000 -D 100000\n\n\n");
}

int zmonkey_args_parser(int argc, char **argv, struct config *config)
{
    char *l_opt_arg;
    char *const short_options = "a:f:n:m:r:c:d:D:j:J:l:L:u:U:y:Y:z:Z:h";
    struct option long_options[] = {
        {"file-prefix", 1, NULL, 'x'},
        {"first_lcore", 1, NULL, 'f'},
        {"core_num", 1, NULL, 'n'},
        {"mbuf_size", 1, NULL, 'm'},
        {"ring_size", 1, NULL, 'r'},
        {"control_port", 1, NULL, 'c'},
        {"l2r_latency", 1, NULL, 'd'},
        {"r2l_latency", 1, NULL, 'D'},
        {"l2r_jitter", 1, NULL, 'j'},
        {"r2l_jitter", 1, NULL, 'J'},
        {"l2r_loss", 1, NULL, 'l'},
        {"r2l_loss", 1, NULL, 'L'},
        {"l2r_dup", 1, NULL, 'u'},
        {"r2l_dup", 1, NULL, 'U'},
        {"r2l_src_mac", 1, NULL, 'y'},
        {"r2l_dst_mac", 1, NULL, 'Y'},
        {"l2r_src_mac", 1, NULL, 'z'},
        {"l2r_dst_mac", 1, NULL, 'Z'},
        {0, 0, 0, 0},
    };

    int c;
    int64_t val;
    while ((c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1)
    {
        switch (c)
        {
        // first lcore
        case 'f':
            val = parse_int(optarg);
            if (val <= 0 || val > MAX_SERVICE_CORE)
            {
                printf("Invalid firstcore value: %ld\n", val);
                zmonkey_usage();
                return -1;
            }
            config->first_lcore = val;
            break;
        // num of core
        case 'n':
            val = parse_int(optarg);
            if (val <= 0 || val > MAX_SERVICE_CORE)
            {
                printf("Invalid num of core value: %ld\n", val);
                zmonkey_usage();
                return -1;
            }
            config->num_service_core = val;
            break;

        // mbuf size
        case 'm':
            val = parse_int(optarg);
            if ((val <= 0) || !powerOfTwo(val))
            {
                printf("Invalid mbuf size: %ld , must be the power of two\n", val);
                zmonkey_usage();
                return -1;
            }
            config->mbuf_size = val;
            break;
        // ring size
        case 'r':
            val = parse_int(optarg);
            if ((val <= 0) || !powerOfTwo(val))
            {
                printf("Invalid ring size: %ld , must be the power of two\n", val);
                zmonkey_usage();
                return -1;
            }
            config->delay_ring_size = val;
            break;
        // control port
        case 'c':
            val = parse_int(optarg);
            if ((val < 1) || (val > 65535))
            {
                printf("Invalid control port: %ld\n", val);
                zmonkey_usage();
                return -1;
            }
            config->ctrl_udp_port = val;
            break;
        // Latency
        case 'd':
            val = parse_int(optarg);
            if (val < 1)
            {
                printf("Invalid latency: %ld\n", val);
                zmonkey_usage();
                return -1;
            }
            config->latency[0] = val;
            break;
        // Latency
        case 'D':
            val = parse_int(optarg);
            if (val < 1)
            {
                printf("Invalid latency: %ld\n", val);
                zmonkey_usage();
                return -1;
            }
            config->latency[1] = val;
            break;

        // Jitter
        case 'j':
            val = parse_int(optarg);
            if (val < 1)
            {
                printf("Invalid jitter: %ld\n", val);
                zmonkey_usage();
                return -1;
            }
            config->jitter[0] = val;
            break;
        // Jitter
        case 'J':
            val = parse_int(optarg);
            if (val < 1)
            {
                printf("Invalid jitter: %ld\n", val);
                zmonkey_usage();
                return -1;
            }
            config->jitter[1] = val;
            break;
        // Loss
        case 'l':
            val = parse_int(optarg);
            if ((val < 1) || (val > 10000))
            {
                printf("Invalid loss rate: %ld\n", val);
                zmonkey_usage();
                return -1;
            }
            config->drop_rate[0] = val;
            break;
        // Loss
        case 'L':
            val = parse_int(optarg);
            if ((val < 1) || (val > 10000))
            {
                printf("Invalid loss rate: %ld\n", val);
                zmonkey_usage();
                return -1;
            }
            config->drop_rate[1] = val;
            break;
        // Duplicate
        case 'u':
            val = parse_int(optarg);
            if ((val < 1) || (val > 10000))
            {
                printf("Invalid duplicate rate: %ld\n", val);
                zmonkey_usage();
                return -1;
            }
            config->dup_rate[0] = val;
            break;
        // Loss
        case 'U':
            val = parse_int(optarg);
            if ((val < 1) || (val > 10000))
            {
                printf("Invalid duplicate rate: %ld\n", val);
                zmonkey_usage();
                return -1;
            }
            config->dup_rate[1] = val;
            break;

        case 'y':
            config->enable_mac_update = 1;
            val = parse_mac(optarg, &config->src_mac[0]);
            if (val != 0)
            {
                printf("Invalid r2l Source MAC address: %s\n", optarg);
                zmonkey_usage();
                return -1;
            }

        case 'Y':
            config->enable_mac_update = 1;
            val = parse_mac(optarg, &config->dst_mac[0]);
            if (val != 0)
            {
                printf("Invalid r2l Destination MAC address: %s\n", optarg);
                zmonkey_usage();
                return -1;
            }

        case 'z':
            config->enable_mac_update = 1;
            val = parse_mac(optarg, &config->src_mac[1]);
            if (val != 0)
            {
                printf("Invalid l2r Source MAC address: %s\n", optarg);
                zmonkey_usage();
                return -1;
            }

        case 'Z':
            config->enable_mac_update = 1;
            val = parse_mac(optarg, &config->dst_mac[1]);
            if (val != 0)
            {
                printf("Invalid l2r Destination MAC address: %s\n", optarg);
                zmonkey_usage();
                return -1;
            }

        case 'a':
            break;
        case 'x':
            break;
        case 'h':
            zmonkey_usage();
            return -1;
        default:
            zmonkey_usage();
            return -1;
        }
    }
    return 0;
}