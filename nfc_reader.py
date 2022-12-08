# Interrogate soak testing MCU over NFC
# Last revised 7 December 2022
# by J. Evan Smith

# ------
# TODO:
# (1) values only update when connection is broken -> sText
# (2) add error handling for reader disconnect
# (3) add error handling for no reader on start

import nfc
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

    parser.add_argument('-f', metavar='file_name', type=str, default='data_output',
        help='File name: use .csv file format')

    args = parser.parse_args()
    verbose = {0: logging.CRITICAL, 1: logging.ERROR, 2: logging.WARNING, 3: logging.INFO, 4: logging.DEBUG}
    logging.basicConfig(format='%(message)s', level=verbose[args.v], stream=sys.stdout)

    return args

def create_file(file_name):
    f = open(file_name,'w')
    f.write('ADC,I2D,Resistance\n')
    f.close()

def connect(file_name):
    rdwr_options = {
        'targets': ['106A'],
        'interval': 0.5,
        'on-startup' : on_startup,
        'on-discover': on_discover,
        'on-connect': lambda tag: False,
        'on-release' : on_release,
    }
    with nfc.ContactlessFrontend('usb:054c') as clf:

        tag = clf.connect(rdwr=rdwr_options)

        print('connected to tag with type and ID:')
        print(tag)
        
        print('attempting to retrieve NDEF text ...')

        while tag.is_present:
    
            sleep(2)
        
            if not tag.ndef:
                print("No tag or NDEF records found!")
                continue

            if tag.ndef.has_changed and tag.is_present:
                f = open(file_name,'a')
                now = datetime.datetime.now()
                print (now.strftime("%Y-%m-%d %H:%M:%S"))
                print(tag.ndef.records[0].text)
                f.write('{} \n'.format(tag.ndef.records[0].text))
                f.close()   
        
        print('tag removed (!)')

def on_startup(targets):
    print('initializing reader ...')
    return targets

def on_discover(tag):
    print('tag found, attempting to connect ...')
    return True

def on_release(tag):
    print('tag moved out of range (!)')
    return tag

def main():
    file_name = args.f
    create_file(file_name)
    try:
        while True:
            connect(file_name)
    except KeyboardInterrupt:
        print('\nend of session')
        print('data saved to: '+file_name)

if __name__ == '__main__':
    args = parse_arguments()
    main()