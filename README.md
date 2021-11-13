# zMonkey 

zMonkey is an open-source 200G network impairment emulator tool to emulate the real-world WAN/DC conditions for your applications.
it can support `90Mpps` packet forwarding with latency, jitter and packet drop simulation.

Latency/Jitter accuracy is around 200 microsecond, packet drop rate accuracy is around 0.1%

zMonkey is based on DPDK and provide a udp based northbound interface for remote controller change the simulated latency/jitter/loss.

```c
============================================================================================================================================

 zMonkey Network Chaos System (by zartbot)


    LeftPort-------->>>>>>>>>>--------->>>>>>>>>              |              <<<<<<<<<--------<<<<<<<<<<---------RightPort
                                                              |
        Latency   :     10000 us                              |                Latency   :         0 us
        Jitter    :         0 us                              |                Jitter    :         0 us
        Loss      :         0 %%                              |                Loss      :         0 %%
        Duplicate :         0 %%                              |                Duplicate :         0 %%
                                                              |
        Total RX  :  68276668 pps --->                        |                <--- Total RX  :   6176950 pps 
        Total TX  :   6176950 pps <---                        |                ---> Total TX  :  68281475 pps 

--------------------------------------------------------------------------------------------------------------------------------------------
Monkey[  0]   4091413 --> LeftPort --> [Drop         0 -->(Q-Depth     40282|Fail         0)--> DeQ    4092640] --> RightPort -->   4092628
Monkey[  1]   4084706 --> LeftPort --> [Drop         0 -->(Q-Depth     39992|Fail         0)--> DeQ    4086496] --> RightPort -->   4086483
Monkey[  2]   4340355 --> LeftPort --> [Drop         0 -->(Q-Depth     42786|Fail         0)--> DeQ    4341760] --> RightPort -->   4341781
Monkey[  3]   4331071 --> LeftPort --> [Drop         0 -->(Q-Depth     42617|Fail         0)--> DeQ    4330944] --> RightPort -->   4330951
Monkey[  4]   4318890 --> LeftPort --> [Drop         0 -->(Q-Depth     42145|Fail         0)--> DeQ    4320416] --> RightPort -->   4320417
Monkey[  5]   4318237 --> LeftPort --> [Drop         0 -->(Q-Depth     44043|Fail         0)--> DeQ    4316896] --> RightPort -->   4316896
Monkey[  6]   4321452 --> LeftPort --> [Drop         0 -->(Q-Depth     43103|Fail         0)--> DeQ    4319104] --> RightPort -->   4319119
Monkey[  7]   4332730 --> LeftPort --> [Drop         0 -->(Q-Depth     46148|Fail         0)--> DeQ    4331424] --> RightPort -->   4331417
Monkey[  8]   4317736 --> LeftPort --> [Drop         0 -->(Q-Depth     43199|Fail         0)--> DeQ    4319424] --> RightPort -->   4319428
Monkey[  9]   4320157 --> LeftPort --> [Drop         0 -->(Q-Depth     43567|Fail         0)--> DeQ    4322016] --> RightPort -->   4322016
Monkey[ 10]   4332787 --> LeftPort --> [Drop         0 -->(Q-Depth     43934|Fail         0)--> DeQ    4332160] --> RightPort -->   4332139
Monkey[ 11]   4319630 --> LeftPort --> [Drop         0 -->(Q-Depth     43216|Fail         0)--> DeQ    4321536] --> RightPort -->   4321538
Monkey[ 12]   4085023 --> LeftPort --> [Drop         0 -->(Q-Depth     41323|Fail         0)--> DeQ    4085216] --> RightPort -->   4085216
Monkey[ 13]   4091710 --> LeftPort --> [Drop         0 -->(Q-Depth     42466|Fail         0)--> DeQ    4090976] --> RightPort -->   4090975
Monkey[ 14]   4329958 --> LeftPort --> [Drop         0 -->(Q-Depth     42092|Fail         0)--> DeQ    4330176] --> RightPort -->   4330186
Monkey[ 15]   4340813 --> LeftPort --> [Drop         0 -->(Q-Depth     42871|Fail         0)--> DeQ    4340288] --> RightPort -->   4340285
--------------------------------------------------------------------------------------------------------------------------------------------
Monkey[  0]    384868 <-- LeftPort <-- [DeQ     384868 <--(Q-Depth         0|Fail         0)<-- Drop         0] <-- RightPort <--    384868
Monkey[  1]    389342 <-- LeftPort <-- [DeQ     389342 <--(Q-Depth         0|Fail         0)<-- Drop         0] <-- RightPort <--    389342
Monkey[  2]    480953 <-- LeftPort <-- [DeQ     480953 <--(Q-Depth         0|Fail         0)<-- Drop         0] <-- RightPort <--    480953
Monkey[  3]    389303 <-- LeftPort <-- [DeQ     389303 <--(Q-Depth         0|Fail         0)<-- Drop         0] <-- RightPort <--    389303
Monkey[  4]    381398 <-- LeftPort <-- [DeQ     381398 <--(Q-Depth         0|Fail         0)<-- Drop         0] <-- RightPort <--    381398
Monkey[  5]    387385 <-- LeftPort <-- [DeQ     387385 <--(Q-Depth         0|Fail         0)<-- Drop         0] <-- RightPort <--    387385
Monkey[  6]    286082 <-- LeftPort <-- [DeQ     286082 <--(Q-Depth         0|Fail         0)<-- Drop         0] <-- RightPort <--    286082
Monkey[  7]    389327 <-- LeftPort <-- [DeQ     389327 <--(Q-Depth         0|Fail         0)<-- Drop         0] <-- RightPort <--    389327
Monkey[  8]    286569 <-- LeftPort <-- [DeQ     286569 <--(Q-Depth         0|Fail         0)<-- Drop         0] <-- RightPort <--    286569
Monkey[  9]    387979 <-- LeftPort <-- [DeQ     387979 <--(Q-Depth         0|Fail         0)<-- Drop         0] <-- RightPort <--    387979
Monkey[ 10]    380284 <-- LeftPort <-- [DeQ     380284 <--(Q-Depth         0|Fail         0)<-- Drop         0] <-- RightPort <--    380284
Monkey[ 11]    388636 <-- LeftPort <-- [DeQ     388636 <--(Q-Depth         0|Fail         0)<-- Drop         0] <-- RightPort <--    388636
Monkey[ 12]    476803 <-- LeftPort <-- [DeQ     476803 <--(Q-Depth         0|Fail         0)<-- Drop         0] <-- RightPort <--    476803
Monkey[ 13]    387778 <-- LeftPort <-- [DeQ     387778 <--(Q-Depth         0|Fail         0)<-- Drop         0] <-- RightPort <--    387778
Monkey[ 14]    389524 <-- LeftPort <-- [DeQ     389524 <--(Q-Depth         0|Fail         0)<-- Drop         0] <-- RightPort <--    389524
Monkey[ 15]    390719 <-- LeftPort <-- [DeQ     390719 <--(Q-Depth         0|Fail         0)<-- Drop         0] <-- RightPort <--    390719
============================================================================================================================================
```

## Compile and install

1.support 100Gbps with 100ms latency, you must change the DPDK MBUF limit in the following file and re-compile DPDK.

Modify `dpdk-21.08/config/rte_config.h`

```c
/* EAL defines */
#define RTE_MAX_HEAPS 32
#define RTE_MAX_MEMSEG_LISTS 512 //128
#define RTE_MAX_MEMSEG_PER_LIST 32768  //8192
#define RTE_MAX_MEM_MB_PER_LIST 131072 //32768
#define RTE_MAX_MEMSEG_PER_TYPE 131072 //32768
#define RTE_MAX_MEM_MB_PER_TYPE 262144 //65536
```

2. git clone and build

```bash
git clone https://github.com/zartbot/zmonkey
cd zmonkey
make

```

## Usage

```bash
sudo ./build/zmonkey -- -h

zmonkey [EAL options] -- <Parameters>
 -f --first_lcore         First lcore used for forwarding thread
 -n --core_num            Number of lcore used for forwarding
 -m --mbuf_size           Number of elements in mbuf
 -r --ring_size           Number of elements in mbuf
 -c --control_port        Remote control udp port(default:6666)

zMonkey chaos config
 -d --l2r_latency         Left  -> Right Delay time [us]
 -D --r2l_latency         Right -> Left Delay time [us]
 -j --l2r_jitter          Left  -> Right Jitter time [us]
 -J --r2l_jitter          Right -> Left Jitter time [us]
 -l --l2r_loss            Left  -> Right Loss rate [%%]
 -L --r2l_loss            Right -> Left Loss rate [%%]
 -u --l2r_dup             Left  -> Right Duplicate rate [%%]
 -U --r2l_dup             Right -> Left Duplicate rate [%%]


Example:

8-Thread to handle 100G/100ms latency with 12.34% packet drop_rate

     zmonkey -- --first_lcore 24 --core_num 8  --mbuf_size 2097152 --l2r_latency 100000 --l2r_loss 1234

16-Thread with 1M element buffer per thread,first lcore start at core.24

     zmonkey -- --first_lcore 24 --core_num 16  --mbuf_size 1048576 --l2r_latency 10000 

Short parameters

     zmonkey -- -f 24 -n 12 -m 2097152 -d 100000 -D 100000

```

## Remote control methods

check the example code in `control/python`

```python
import socket
import random
import time


udpSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
monkeyHost = ('127.0.0.1',6666)

#  instr
#  1: config latency
#  2: config jitter in micro seconds
#  3: config packet drop rate in %%(1/10000)
#  4: config packet duplication rate in %%(1/10000)
# 
#  direction
#  0: left-->right
#  1: right-->left
# 
#  value uint64_t

instr = 3
direction = 0
value = 1234

sendData = str(instr) +"," + str(direction) +","+ str(value) 
udpSocket.sendto(sendData.encode(),monkeyHost)
```


