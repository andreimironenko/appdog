#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from libappdog import client
import os
from time import sleep
import signal

is_flag_set = False

def signal_handler(signal_number, frame):
    del frame
    global is_flag_set 
    if signal_number == signal.SIGUSR1:
        is_flag_set = True 
        print("Received signal SIGUSR1")

if __name__ == "__main__":
    signal.signal(signal.SIGUSR1, signal_handler)
    awd = client(os.getpid(), 0)
    ret = awd.activateL(5*1000000000, signal.SIGUSR1, 0)
    print(ret)

    while(not is_flag_set):
        print("wait for SIGUSR1 signal, sleep for 1 sec")
        sleep(1)
    print("Test is passed, quiting ...")



