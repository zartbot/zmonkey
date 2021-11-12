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
