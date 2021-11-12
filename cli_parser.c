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

void zmonkey_usage()
{
    printf("\n\nzmonkey [EAL options] -- <Parameters>\n");
    printf(" -f --first_lcore         First lcore used for forwarding thread\n"
           " -n --core_num            Number of lcore used for forwarding\n"
           " -m --mbuf_size           Number of elements in mbuf\n"
           " -r --ring_size           Number of elements in mbuf\n"
           " -c --control_port        Remote control udp port(default:6666)\n"
           "\nzMonkey chaos config\n"
           " -d --l2r_latency         Left  -> Right Delay time [us]\n"
           " -D --r2l_latency         Right -> Left Delay time [us]\n"
           " -j --l2r_jitter          Left  -> Right Jitter time [us]\n"
           " -J --r2l_jitter          Right -> Left Jitter time [us]\n"
           " -l --l2r_loss            Left  -> Right Loss rate [%%%%]\n"
           " -L --r2l_loss            Right -> Left Loss rate [%%%%]\n"
           " -u --l2r_dup             Left  -> Right Duplicate rate [%%%%]\n"
           " -U --r2l_dup             Right -> Left Duplicate rate [%%%%]\n\n\n"
           "Example:\n\n"
           "8-Thread to handle 100G/100ms latency with 12.34%% packet drop_rate\n\n"
           "     zmonkey -- --first_lcore 24 --core_num 8  --mbuf_size 2097152 --l2r_latency 100000 --l2r_loss 1234\n"
           "16-Thread with 1M element buffer per thread,first lcore start at core.24\n\n"
           "     zmonkey -- --first_lcore 24 --core_num 16  --mbuf_size 1048576 --l2r_latency 10000 \n"
           "Short parameters\n\n"
           "     zmonkey -- -f 24 -n 12 -m 2097152 -d 100000 -D 100000\n\n\n");
}

int zmonkey_args_parser(int argc, char **argv, struct config *config)
{
    char *l_opt_arg;
    char *const short_options = "a:f:n:m:r:c:d:D:j:J:l:L:u:U:h";
    struct option long_options[] = {
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
            if ((val < 1) || (val> 65535))
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
        case 'a':
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
