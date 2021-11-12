import socket
import random
import time


udpSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
monkeyHost = ('127.0.0.1',6666)

for instr in range(1,5):
    for direction in range(0,2):
        value = random.randrange(1,100000)
        sendData = str(instr) +"," + str(direction) +","+ str(value) 
        udpSocket.sendto(sendData.encode(),monkeyHost)
        time.sleep(1)