# Interrogate soak testing MCU over NFC
# Last revised 12 December 2022
# by J. Evan Smith

# ------
# TODO:
# (1) list index for tag.ndef.records[0] out of range on first run
# (2) Reduce to text rather than text+mime —> see ndeft2t_mod_example_1.c 
# (3) add error handling for reader disconnect
# (4) add error handling for no reader on start

import nfc
import plotext as pltx
import numpy as np

import sys
import datetime
import argparse

from scipy.optimize import curve_fit

import ndef
from ndef import TextRecord

import time
from time import sleep
import logging
from logging import critical, error, info, warning, debug

flagx = True

def parse_arguments():
    parser = argparse.ArgumentParser(description='Arguments get parsed via --commands')
    
    parser.add_argument('-v', metavar='verbosity', type=int, default=2,
        help='Verbosity of logging: 0 -critical, 1- error, 2 -warning, 3 -info, 4 -debug')

    parser.add_argument('-f', metavar='file_name', type=str, default='data_output',
        help='File name: use .csv file format')

    parser.add_argument('-t', metavar='period', type=int, default=5,
        help='Period between subsequent measurements')    

    args = parser.parse_args()
    verbose = {0: logging.CRITICAL, 1: logging.ERROR, 2: logging.WARNING, 3: logging.INFO, 4: logging.DEBUG}
    logging.basicConfig(format='%(message)s', level=verbose[args.v], stream=sys.stdout)

    return args

def create_file(file_name):
    f = open(file_name,'w')
    f.write('ADC,I2D,Resistance\n')
    f.close()

def waveform(x,a,b,c):
    return a*np.sin(b*x)+c

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
    
            pltx.clear_figure()
            count_down(args)
        
            if not tag.ndef:
                print("No tag or NDEF records found!")
                continue

            if tag.is_present: #and tag.ndef.has_changed
                
                #try:
                    f = open(file_name,'a')
                    now = datetime.datetime.now()
                    print ("data from " + now.strftime("%Y-%m-%d %H:%M:%S"))

                    for records in tag.ndef.records:
                        print(records)
                
                    a = np.array(tag.ndef.records[0].data) 
                    b = np.array(tag.ndef.records[1].data)
                    y1 = np.zeros(200)

                    print('batch 1')
                    print('sending acknolwedge ...')

                    tag.ndef.records = [TextRecord("a")]
                
                    sleep(1)
                    assert tag.ndef.has_changed is True

                    for records in tag.ndef.records:
                        print(records)

                    c = np.array(tag.ndef.records[0].data) 
                    d = np.array(tag.ndef.records[1].data)
                    y2 = np.zeros(200)

                    print('batch 2')
                    print('sending acknolwedge ...')

                    tag.ndef.records = [TextRecord("b")]

                    sleep(1)
                    assert tag.ndef.has_changed is True

                    for k in np.arange(0,100,1):
                        y1[k] = a[2*k]+256*a[2*k+1]
                        y1[k+100] = b[2*k]+256*b[2*k+1]
                        y2[k] = c[2*k]+256*c[2*k+1]
                        y2[k+100] = d[2*k]+256*d[2*k+1]

                    x = np.linspace(0,len(y1)-1,len(y1))
                    param1, param_cov1 = curve_fit(waveform,x,y1,[1500, 0.05, 2000])
                    param2, param_cov2 = curve_fit(waveform,x,y2,[1500, 0.05, 2000])

                    #f.write('{} \n'.format(tag.ndef.records[0].text))
                    f.close()   

                    pltx.scatter(y1,color='white',marker='braille')
                    pltx.title('response 1')
                    pltx_style()
                    pltx.show()
                    pltx.clear_figure()

                    pltx.scatter(y2,color='white',marker='braille')
                    pltx.title('response 2')
                    pltx_style()
                    pltx.show()
                    pltx.clear_figure()

                    pltx.scatter(y1,color='red+',marker='braille')
                    pltx.scatter(y2,color='blue+',marker='braille')
                    pltx.title('comparison')
                    pltx_style()
                    pltx.show()
                    pltx.clear_figure()

                    z1 = param1[0]*np.sin(param1[1]*x)+param1[2]
                    z2 = param2[0]*np.sin(param2[1]*x)+param2[2]

                    pltx.scatter(z1,color='red+',marker='braille')
                    pltx.scatter(z2,color='blue+',marker='braille')
                    pltx.title('comparison')
                    pltx_style()
                    pltx.show()
                    pltx.clear_figure()

                    print('parameters from driving wave:')
                    print(param1)

                    print('parameters from response wave:')
                    print(param2)

                    vch1 = param1[0]/(2**12)*1.6
                    vch2 = param2[0]/(2**12)*1.6

                    I = (vch1-vch2)/97.8e3
                    Z = vch2/I

                    print('impedance estimated to be')
                    print(Z)

                    #print(param)
                    #z = param[0]*np.sin(param[1]*x)+param[2]
                    #pltx.plot(x,z,color='red',marker='braille')
                    #pltx.title('curve fit results')
                    #pltx_style()
                    #pltx.show()
                    #pltx.clear_figure()
                
                #except:
                #    print('something went wrong! trying again ...')
                #    continue
                

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

def pltx_style():
    pltx.canvas_color('black')
    pltx.axes_color('black')
    pltx.ticks_color('white')
    pltx.xlabel('samples')
    th = pltx.terminal_height()
    tw = pltx.terminal_width()
    pltx.plot_size(0.6*tw, 0.4*th)
    pltx.ylim(0,4096)

def test_plot():
    x = np.arange(0,12.56,0.1)
    y = np.sin(x)

    pltx.plot(x,y,label='sin',color='red',marker='braille')
    pltx_style()
    pltx.show()

def count_down(args):
    N = args.t
    for i in range(N):
        sleep(1)
        print(f"next measurement in {N-i:d} seconds ...", end="\r")    

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