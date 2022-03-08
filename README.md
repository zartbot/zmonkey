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
        Latency   :     10000 us                              |                Latency   :     10000 us
        Jitter    :         0 us                              |                Jitter    :         0 us
        Loss      :         0 %%                              |                Loss      :         0 %%
        Duplicate :         0 %%                              |                Duplicate :         0 %%
                                                              |
        Total RX  :   8537641 pps  --->                       |                <--- Total RX  :  12560523 pps 
        Total TX  :  12561167 pps  <---                       |                ---> Total TX  :   8537849 pps 
                                                              |
        Total TX  :     63058 Mbps <---                       |                ---> Total TX  :     94972 Mbps 

--------------------------------------------------------------------------------------------------------------------------------------------
Monkey[  0]    527920 --> LeftPort --> [Drop         0 -->(Q-Depth      5267|Fail         0)--> DeQ     527904] --> RightPort -->    527919
Monkey[  1]    527922 --> LeftPort --> [Drop         0 -->(Q-Depth      5264|Fail         0)--> DeQ     527904] --> RightPort -->    527925
Monkey[  2]    527941 --> LeftPort --> [Drop         0 -->(Q-Depth      5267|Fail         0)--> DeQ     527936] --> RightPort -->    527940
Monkey[  3]    527924 --> LeftPort --> [Drop         0 -->(Q-Depth      5248|Fail         0)--> DeQ     527936] --> RightPort -->    527922
Monkey[  4]    322660 --> LeftPort --> [Drop         0 -->(Q-Depth      3175|Fail         0)--> DeQ     322752] --> RightPort -->    322762
Monkey[  5]    754911 --> LeftPort --> [Drop         0 -->(Q-Depth      2201|Fail         0)--> DeQ     754912] --> RightPort -->    754909
Monkey[  6]    552003 --> LeftPort --> [Drop         0 -->(Q-Depth       752|Fail         0)--> DeQ     552032] --> RightPort -->    552025
Monkey[  7]    552007 --> LeftPort --> [Drop         0 -->(Q-Depth      1142|Fail         0)--> DeQ     552000] --> RightPort -->    552001
Monkey[  8]    754256 --> LeftPort --> [Drop         0 -->(Q-Depth      2114|Fail         0)--> DeQ     754272] --> RightPort -->    754256
Monkey[  9]    322576 --> LeftPort --> [Drop         0 -->(Q-Depth      3130|Fail         0)--> DeQ     322656] --> RightPort -->    322669
Monkey[ 10]    527904 --> LeftPort --> [Drop         0 -->(Q-Depth      5266|Fail         0)--> DeQ     527904] --> RightPort -->    527904
Monkey[ 11]    527939 --> LeftPort --> [Drop         0 -->(Q-Depth      5269|Fail         0)--> DeQ     527936] --> RightPort -->    527938
Monkey[ 12]    527906 --> LeftPort --> [Drop         0 -->(Q-Depth      5261|Fail         0)--> DeQ     527904] --> RightPort -->    527905
Monkey[ 13]    527932 --> LeftPort --> [Drop         0 -->(Q-Depth      5248|Fail         0)--> DeQ     527936] --> RightPort -->    527935
Monkey[ 14]    527916 --> LeftPort --> [Drop         0 -->(Q-Depth      5241|Fail         0)--> DeQ     527936] --> RightPort -->    527916
Monkey[ 15]    527924 --> LeftPort --> [Drop         0 -->(Q-Depth      5269|Fail         0)--> DeQ     527904] --> RightPort -->    527923
--------------------------------------------------------------------------------------------------------------------------------------------
Monkey[  0]    786318 <-- LeftPort <-- [DeQ     786336 <--(Q-Depth      7956|Fail         0)<-- Drop         0] <-- RightPort <--    786304
Monkey[  1]    787736 <-- LeftPort <-- [DeQ     787744 <--(Q-Depth      8060|Fail         0)<-- Drop         0] <-- RightPort <--    787575
Monkey[  2]    982515 <-- LeftPort <-- [DeQ     982496 <--(Q-Depth      9670|Fail         0)<-- Drop         0] <-- RightPort <--    982476
Monkey[  3]    785124 <-- LeftPort <-- [DeQ     785120 <--(Q-Depth      7816|Fail         0)<-- Drop         0] <-- RightPort <--    785047
Monkey[  4]    786164 <-- LeftPort <-- [DeQ     786144 <--(Q-Depth      7670|Fail         0)<-- Drop         0] <-- RightPort <--    786208
Monkey[  5]    564890 <-- LeftPort <-- [DeQ     564896 <--(Q-Depth      1653|Fail         0)<-- Drop         0] <-- RightPort <--    565281
Monkey[  6]    863260 <-- LeftPort <-- [DeQ     863264 <--(Q-Depth      1024|Fail         0)<-- Drop         0] <-- RightPort <--    862875
Monkey[  7]    752708 <-- LeftPort <-- [DeQ     752736 <--(Q-Depth      1460|Fail         0)<-- Drop         0] <-- RightPort <--    752707
Monkey[  8]    750114 <-- LeftPort <-- [DeQ     750112 <--(Q-Depth      2153|Fail         0)<-- Drop         0] <-- RightPort <--    750015
Monkey[  9]    784226 <-- LeftPort <-- [DeQ     784224 <--(Q-Depth      7923|Fail         0)<-- Drop         0] <-- RightPort <--    784162
Monkey[ 10]    588525 <-- LeftPort <-- [DeQ     588512 <--(Q-Depth      5472|Fail         0)<-- Drop         0] <-- RightPort <--    588200
Monkey[ 11]    785942 <-- LeftPort <-- [DeQ     785952 <--(Q-Depth      8097|Fail         0)<-- Drop         0] <-- RightPort <--    785941
Monkey[ 12]    785343 <-- LeftPort <-- [DeQ     785344 <--(Q-Depth      7563|Fail         0)<-- Drop         0] <-- RightPort <--    785388
Monkey[ 13]    984020 <-- LeftPort <-- [DeQ     984032 <--(Q-Depth      9791|Fail         0)<-- Drop         0] <-- RightPort <--    984100
Monkey[ 14]    787396 <-- LeftPort <-- [DeQ     787392 <--(Q-Depth      7673|Fail         0)<-- Drop         0] <-- RightPort <--    787414
Monkey[ 15]    786886 <-- LeftPort <-- [DeQ     786880 <--(Q-Depth      8006|Fail         0)<-- Drop         0] <-- RightPort <--    786830
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


 -y --r2l_src_mac         Right -> Left Source      MAC[xx:xx:xx:xx:xx:xx]
 -Y --r2l_dst_mac         Right -> Left Destination MAC[xx:xx:xx:xx:xx:xx]
 -z --l2r_src_mac         Left  -> Right Source      MAC[xx:xx:xx:xx:xx:xx]
 -Z --l2r_dst_mac         Left  -> Right Destination MAC[xx:xx:xx:xx:xx:xx]


Example:

8-Thread to handle 100G/100ms latency with 12.34% packet drop_rate

     zmonkey -- --first_lcore 24 --core_num 8  --mbuf_size 2097152 --l2r_latency 100000 --l2r_loss 1234

16-Thread with 1M element buffer per thread,first lcore start at core.24

     zmonkey -- --first_lcore 24 --core_num 16  --mbuf_size 1048576 --l2r_latency 10000 

Short parameters

     zmonkey -- -f 24 -n 12 -m 2097152 -d 100000 -D 100000

Modify MAC address for Router testing

     zmonkey -- -y aa:aa:bb:1b:cc:cc  -Y de:ad:be:ef:aa:bb -z ca:fe:be:ef:00:01 -Z ca:fe:be:ef:00:02

```
## Multi-instance 

For example, we wants to build multiple bridge to emulate latency, the device PCIe id shows as below:

```bash
sudo dpdk-devbind.py -s

Network devices using DPDK-compatible driver
============================================
0000:af:00.0 'Ethernet Controller E810-C for QSFP 1592' drv=vfio-pci unused=ice
0000:af:00.1 'Ethernet Controller E810-C for QSFP 1592' drv=vfio-pci unused=ice
...
0000:86:00.0 'MT28800 Family [ConnectX-5 Ex] 1019' if=ens17f0 drv=mlx5_core unused=vfio-pci 
0000:86:00.1 'MT28800 Family [ConnectX-5 Ex] 1019' if=ens17f1 drv=mlx5_core unused=vfio-pci 
```

we could use EAL parameters `-a` allow dedicated device and `--file-prefix` for difference apps, please also change the `first_lcore ` and `control_port` for difference applications.
### Instance-1  [0000:af:00.0  <--> 0000:86:00.0 on core 29~32]

```bash
sudo ./build/zmonkey -a 0000:af:00.0 -a 0000:86:00.0 --file-prefix zm1 --  --first_lcore 24  --control_port 5555 --core_num 4  --mbuf_size 1048576 --l2r_latency 50000 --r2l_latency 50000
```

### Instance-2 with [0000:af:00.1  <--> 0000:86:00.1 on core 29~32]

```bash
sudo ./build/zmonkey -a 0000:af:00.1 -a 0000:86:00.1 --file-prefix zm2 --  --first_lcore 29 --control_port 7777 --core_num 4  --mbuf_size 1048576 --l2r_latency 50000 --r2l_latency 50000 
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


