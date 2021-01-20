#!/usr/bin/env python3
# coding=utf-8

import os, sys, argparse, time, json
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *


# import pyaudio
# pa = pyaudio.PyAudio()
# chosen_device_index = -1
# for x in range(0, pa.get_device_count()):
# 	info = pa.get_device_info_by_index(x)
# 	print(pa.get_device_info_by_index(x))
# 	if info["name"] == "pulse":
# 		chosen_device_index = info["index"]
# 		print("Chosen index: ", chosen_device_index)
#
# stream = pa.open(format=pyaudio.paInt16, channels=1, rate=44100, input_device_index=chosen_device_index, input=True,
#                  output=False, frames_per_buffer=44100)
# frames = []
# for i in range(3):
# 	data = stream.read(44100)
# 	frames.append(data)
#
# stream.stop_stream()
# stream.close()
#
# play_stream = pa.open(format=pyaudio.paInt16, channels=1, rate=44100, output=True)
# for frame in frames:
# 	play_stream.write(frame)
#
# pa.terminate()
#
# aa=5

# import pyaudio
# import wave
# import sys
#
# # open the file for reading.
# wf = wave.open('/Users/xuancong/Desktop/test.wav', 'rb')
#
# # create an audio object
# p = pyaudio.PyAudio()
#
# if False:
# 	stream = p.open(format=
# 	                p.get_format_from_width(wf.getsampwidth()),
# 	                channels=wf.getnchannels(),
# 	                rate=wf.getframerate(),
# 	                output=True)
# 	stream.write(wf.readframes(4410000))
# else:
# 	# open stream based on the wave object which has been input.
# 	stream = p.open(format = pyaudio.paFloat32,
# 	                channels = 1,
# 	                rate = wf.getframerate(),
# 	                output = True)
#
# 	# read data (based on the chunk size)
# 	data = wf.readframes(4410000)
# 	aa=np.frombuffer(data, dtype=np.int16)
# 	bb=aa.reshape([1438532//2, 2]).astype(np.float32)/32768.
#
# 	# play stream (looping from beginning of file to the end)
# 	stream.write(bb[:,0].tobytes())
#
# # cleanup stuff.
# stream.close()
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
	parser.add_argument('--fullscreen', '-fs', help='draw full screen', action='store_true')
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
		verbose = verbose  # Print running statistics (latency, fps, ...)
	)

	if list_devices:
		print(json.dumps(tv.stream_reader.infos, indent=1))
		sys.exit(0)

	tv.start()

