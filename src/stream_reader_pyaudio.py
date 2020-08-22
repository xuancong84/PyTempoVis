import os, sys, pyaudio, threading, math, time, code
import numpy as np
from collections import deque


class Stream_Reader:
	"""
	The Stream_Reader continuously reads data from a selected sound source using PyAudio

	Arguments:

		device: int or None:    Select which audio stream to read .
		rate: float or None:    Sample rate to use. Defaults to something supported.
		updatesPerSecond: int:  How often to record new data.

	"""

	def __init__(self,
	             device = None,
	             rate = None,
	             updates_per_second = 1000,
	             n_channels = 1,
	             sample_type = np.float32,
	             buffer_seconds = 1.0,
	             verbose = False):

		self.verbose = verbose
		self.sample_type = sample_type
		self.pa = pyaudio.PyAudio()

		self.device = self.pa.get_default_input_device_info()['index'] if device is None else device
		self.rate = self.pa.get_default_input_device_info()['defaultSampleRate'] if rate is None else rate

		self.update_window_n_frames = int((self.rate / updates_per_second + 1)//2*2)
		self.updates_per_second = self.rate / self.update_window_n_frames
		self.infos = {'default_device': self.pa.get_default_input_device_info()['index'],
		              'device_list': [self.pa.get_device_info_by_index(i) for i in range(self.pa.get_device_count())]}
		self.info = self.pa.get_device_info_by_index(self.device)
		self.array_length = int(self.rate * buffer_seconds + 0.5)
		self.lock = threading.Lock()

		type_map = {
			np.float32 : pyaudio.paFloat32,      #: 32 bit float
			np.int32 : pyaudio.paInt32,        #: 32 bit int
			np.int16 : pyaudio.paInt16,        #: 16 bit int
			np.int8 : pyaudio.paInt8,         #: 8 bit int
			np.uint8 : pyaudio.paUInt8        #: 8 bit unsigned int
		}

		self.stream = self.pa.open(
			start = False,
			format = type_map[sample_type],
			channels = n_channels,
			rate = self.rate,
			input = True,
			frames_per_buffer = self.update_window_n_frames,
			stream_callback = self.non_blocking_stream_read)

	def __del__(self):
		self.stream.close()
		self.pa.terminate()

	def non_blocking_stream_read(self, in_data, frame_count, time_info, status):
		# when multiple channel, in_data is interleaved
		# print(frame_count, time_info)
		# code.interact(local=dict(globals(), **locals()) )
		new_wav = np.frombuffer(in_data, dtype=self.sample_type).reshape([frame_count,self.stream._channels]).T
		# if False:
		# 	wav_data = np.concatenate([self.wav_data[:, frame_count:], new_wav], axis=1)
		# 	wav_time = time_info['input_buffer_adc_time'] + frame_count / self.rate
		# 	self.wav_data, self.wav_time = wav_data, wav_time
		# else:
		# 	# this is at least 4 times much faster, but has data race issue

		self.lock.acquire()
		try:
			self.wav_data[:, :-frame_count] = self.wav_data[:, frame_count:]
			self.wav_data[:, -frame_count:] = new_wav
			self.wav_time = time_info['input_buffer_adc_time']+frame_count/self.rate
			if self.stream_start_time is None:
				self.stream_start_time = time_info['input_buffer_adc_time']
		finally:
			self.lock.release()

		return in_data, pyaudio.paContinue

	def stream_start(self):
		self.wav_data = np.zeros([self.stream._channels, self.array_length], dtype=self.sample_type)
		self.wav_time = None

		self.stream.start_stream()
		self.stream_start_time = None

	def stream_stop(self):
		self.stream.stop_stream()
