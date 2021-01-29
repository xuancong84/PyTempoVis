#!/usr/bin/env python3
# coding=utf-8

import os, sys, argparse, time, json
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *


# import numpy as np
# import pyaudio
# p = pyaudio.PyAudio()
# chosen_device_index = -1
# for x in range(0, p.get_device_count()):
# 	info = p.get_device_info_by_index(x)
# 	print(p.get_device_info_by_index(x))
# 	if info["name"] == "default":
# 		chosen_device_index = info["index"]
# 		print("Chosen index: ", chosen_device_index)
#
# stream = p.open(format=pyaudio.paInt16, channels=32, rate=44100, input=True,
#                  output=False, frames_per_buffer=44100)
# frames = [stream.read(44100) for i in range(5)]
#
# stream.stop_stream()
# stream.close()
#
# def play(chs, frames):
# 	play_stream = p.open(format=pyaudio.paInt16, channels=len(chs), rate=44100, output=True)
# 	[play_stream.write(np.frombuffer(data, dtype=np.int16).reshape([44100, 32])[:,chs].tobytes()) for data in frames]
#
# play([0,1], frames)
#
# p.terminate()


# print('PID=', os.getpid())

if __name__ == '__main__':

	parser = argparse.ArgumentParser(usage='$0 [options]', description='Music Tempo Visualization',
	                                 formatter_class=argparse.ArgumentDefaultsHelpFormatter)
	parser.add_argument('--fps', '-fps', help='update rate in frames-per-second', type=float, default=100)
	parser.add_argument('--device', '-d', help='audio device index', type=int, default=None)
	parser.add_argument('--num-channels', '-nc', help='number of recording channels', type=int, default=2)
	parser.add_argument('--list-devices', '-ls', help='list recording devices and quit', action='store_true')
	parser.add_argument('--sample-rate', '-sr', help='audio recording sampling rate', type=int, default=44100)
	parser.add_argument('--tempo-buffer-seconds', '-tbs', help='compute tempo from last N seconds audio', type=int, default=12)
	parser.add_argument('--tempo-calc-interval', '-tci', help='re-compute tempo every N seconds', type=int, default=5)
	parser.add_argument('--fullscreen', '-f', help='draw full screen', action='store_true')
	parser.add_argument('--verbose', '-v', help='verbose mode', action='store_true')
	# nargs='?': optional positional argument; action='append': multiple instances of the arg; type=; default=
	opt = parser.parse_args()
	globals().update(vars(opt))

	from src.visualizer import *

	tv = TempoVis(
		device = device,  # Manually play with this (int) if you don't see anything
		rate = sample_rate,  # Audio samplerate, None uses the default source settings
		n_channels = num_channels, # Number of recording channels
		tempo_buffer_seconds = tempo_buffer_seconds,
		tempo_calc_interval = tempo_calc_interval,
		updates_per_second = fps,  # How often to read the audio stream for new data
		full_screen = fullscreen,
		verbose = verbose  # Print running statistics (latency, fps, ...)
	)

	if list_devices:
		print(json.dumps(tv.stream_reader.infos, indent=1))
		sys.exit(0)

	tv.start()

