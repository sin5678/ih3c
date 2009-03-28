#!/usr/bin/env python
# encoding : utf-8

from os import system

def ControlSvr(Command, SvrName):
    command = 'net '
    command += Command
    command += ' '
    command += SvrName
    system (command)

from socket import *

def VerifyNetworkStatus(addr, port):
    tcpsock = socket (AF_INET, SOCK_STREAM)
    return tcpsock.connect_ex ((addr, port)) == 0

import sys
import logging
import logging.handlers
from time import sleep

class Unique(logging.Filter):
    """Messages are allowed through just once.
    The 'message' includes substitutions, but is not formatted by the
    handler. If it were, then practically all messages would be unique!"""
    def __init__(self, name=""):
        logging.Filter.__init__(self, name)
        self.reset()
    def reset(self):
        """Act as if nothing has happened."""
        self.__logged = {}
    def filter(self, rec):
        """logging.Filter.filter performs an extra filter on the name."""
        return logging.Filter.filter(self, rec) and self.__is_first_time(rec)
    def __is_first_time(self, rec):
        """Emit a message only once."""
        msg = rec.msg %(rec.args)
        if msg in self.__logged:
            self.__logged[msg] += 1
            return False
        else:
            self.__logged[msg] = 1
            return True

LOG_FILENAME = 'd:/tmp.txt'
print 'NetWatchdog'
# Set up a specific logger with our desired output level
logger = logging.getLogger()
logger.setLevel(logging.DEBUG)

# file output
handler = logging.handlers.RotatingFileHandler(
              LOG_FILENAME, maxBytes=1000)
formatter = logging.Formatter('%(asctime)s %(message)s')
handler.setFormatter(formatter)
handler.setLevel (logging.NOTSET)
logger.addHandler(handler)

# console output
ch = logging.StreamHandler()
ch.setLevel (logging.NOTSET)
ch.setFormatter(formatter)
logger.addHandler(ch)

unique = Unique()
logger.addFilter(unique)

try:
    while (True):
        sleep (120)
        if VerifyNetworkStatus('www.sysu.edu.cn', 80) == False :
            logger.info ( 'network down')
            ControlSvr ('stop', 'myh3c')
            sleep(5)
            ControlSvr('start', 'myh3c')
            unique.reset()
        else:
            logger.info ( 'network status normal')
except:
        logger.exception ('Ooh!~', sys.exc_info())