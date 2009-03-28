#!/usr/bin/env python
# -*- coding: gb2312 -*-
#http://blog.chinaunix.net/u1/43271/showart_1102599.html
#命令行加强

def getMyIp ():
    import socket
    return socket.gethostbyname(socket.gethostname())

def updateMyIp ():
    import urllib2
    try:
        auth = urllib2.HTTPBasicAuthHandler()
        url = 'http://www.3322.org/dyndns/update?system=dyndns&hostname=novateam.8800.org&myip=' + getMyIp()
        auth.add_password('Bentium DynDNS NIC', url, 'username', 'password')
        opener = urllib2.build_opener(auth, urllib2.CacheFTPHandler)
        urllib2.install_opener(opener)
        urllib2.urlopen(url) #read ()
    except:
        import sys
        print sys.exc_info()
        pass

from time import sleep
print 'IpUpdater'
while (True):
    updateMyIp ()
    sleep (60*10)