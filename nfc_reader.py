# Interrogate soak testing MCU over NFC
# Last revised 7 December 2022
# by J. Evan Smith

# ------
# TODO:
# (1) duplicate records?
# (2) add error handling for reader disconnect
# (3) add error handling for no reader on start

import nfc
from nfc.clf import RemoteTarget
import ndef

import sys
import datetime
import argparse

import time
from time import sleep
import logging
from logging import critical, error, info, warning, debug

def parse_arguments():
    parser = argparse.ArgumentParser(description='Arguments get parsed via --commands')
    parser.add_argument('-v', metavar='verbosity', type=int, default=2,
        help='Verbosity of logging: 0 -critical, 1- error, 2 -warning, 3 -info, 4 -debug')

    args = parser.parse_args()
    verbose = {0: logging.CRITICAL, 1: logging.ERROR, 2: logging.WARNING, 3: logging.INFO, 4: logging.DEBUG}
    logging.basicConfig(format='%(message)s', level=verbose[args.v], stream=sys.stdout)
    
    return args


def connect():
    rdwr_options = {
        'targets': ['106A'],
        'on-startup': on_startup,
        'on-connect': on_connect,
        'on-release': on_release,
    }
    with nfc.ContactlessFrontend('usb:054c') as clf:
        #tag = clf.connect(rdwr=rdwr_options)
        
        while True:
            target = clf.sense(RemoteTarget('106A'))
            if target is None:
                sleep(0.1) 
                continue
            
            sleep(9)
            tag = nfc.tag.activate(clf, target)
            sleep(1)
        
            f = open('data_1.csv','a')
    
            if not tag.ndef:
                print("No tag or NDEF records found!")
                continue
        
            now = datetime.datetime.now()
            print (now.strftime("%Y-%m-%d %H:%M:%S"))
            print('{}'.format(tag.ndef.records[0].text))
            f.write('{}\n'.format(tag.ndef.records[0].text)) 

            f.close()   
        

def on_startup(targets):
    print("startup callback")

def on_connect(tag):
    print("on connect callback")

def on_release(tag):
    print("on release callback")

def main():
    f = open('data_1.csv','w')
    f.write('data header\n')
    f.close()
    connect()

if __name__ == '__main__':
    args = parse_arguments()
    main()