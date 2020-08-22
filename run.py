#!/usr/bin/env python3
# coding=utf-8

import os, sys, argparse, time, json
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
import ctypes


if __name__ == '__main__':

	parser = argparse.ArgumentParser(usage='$0 [options]', description='Music Tempo Visualization',
	                                 formatter_class=argparse.ArgumentDefaultsHelpFormatter)
	parser.add_argument('--fps', '-fps', help='update rate in frames-per-second', type=float, default=100)
	parser.add_argument('--device', '-d', help='audio device index', type=int, default=None)
	parser.add_argument('--num-channels', '-nc', help='number of recording channels', type=int, default=2)
	parser.add_argument('--list-devices', '-ls', help='list recording devices and quit', action='store_true')
	parser.add_argument('--sample-rate', '-sr', help='audio recording sampling rate', type=int, default=44100)
	parser.add_argument('--fullscreen', '-fs', help='draw full screen', action='store_true')
	parser.add_argument('--verbose', '-v', help='verbose mode', action='store_true')
	# nargs='?': optional positional argument; action='append': multiple instances of the arg; type=; default=
	opt = parser.parse_args()
	globals().update(vars(opt))

	from src.stream_analyzer import *

	tv = TempoVis(
		device = device,  # Manually play with this (int) if you don't see anything
		rate = sample_rate,  # Audio samplerate, None uses the default source settings
		n_channels = num_channels, # Number of recording channels
		updates_per_second = fps,  # How often to read the audio stream for new data
		verbose = verbose  # Print running statistics (latency, fps, ...)
	)

	if list_devices:
		print(json.dumps(tv.stream_reader.infos, indent=1))
		sys.exit(0)

	tv.start()

