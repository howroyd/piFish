#!/usr/bin/python3
import RPi.GPIO as gpio
import argparse

# Inspect user input arguments
def _parse_commandline():
    # Define the parser
    parser = argparse.ArgumentParser(description='Switch GPIO')
    
    # Define aguments
    parser.add_argument('pin', type=int)
    parser.add_argument('-on', action="store_true", default=False)
    parser.add_argument('-off', action="store_true", default=False)

    # Return what was argued
    return parser.parse_args()

args = _parse_commandline()

state = False
if args.on:
    state = True
    print(args.pin, "on")
if args.off:
    state = False
    print(args.pin, "off")

gpio.setwarnings(False)
gpio.setmode(gpio.BCM)
gpio.setup(args.pin, gpio.OUT, initial=0)
gpio.output(args.pin, state)
