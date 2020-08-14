import os, sys, pyaudio, code
import numpy as np
import time, sys, math
from collections import deque

from src.utils import *


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
	             verbose = False):

		self.verbose = verbose
		self.sample_type = sample_type
		self.pa = pyaudio.PyAudio()

		# Temporary variables #hacks!
		self.update_window_n_frames = 1024  # Don't remove this, needed for device testing!
		self.data_buffer = None

		self.device = self.pa.get_default_input_device_info()['index'] if device is None else device
		self.rate = self.pa.get_default_input_device_info()['defaultSampleRate'] if rate is None else rate

		self.update_window_n_frames = round_up_to_even(self.rate / updates_per_second)
		self.updates_per_second = self.rate / self.update_window_n_frames
		self.infos = {'default_device': self.pa.get_default_input_device_info()['index'],
		              'device_list': [self.pa.get_device_info_by_index(i) for i in range(self.pa.get_device_count())]}
		self.info = self.pa.get_device_info_by_index(self.device)
		self.new_data = False

		type_map = {
			np.float32 : pyaudio.paFloat32,      #: 32 bit float
			np.int32 : pyaudio.paInt32,        #: 32 bit int
			np.int16 : pyaudio.paInt16,        #: 16 bit int
			np.int8 : pyaudio.paInt8,         #: 8 bit int
			np.uint8 : pyaudio.paUInt8        #: 8 bit unsigned int
		}

		if self.verbose:
			self.data_capture_delays = deque(maxlen=32)
			self.num_data_captures = 0

		self.stream = self.pa.open(
			format = type_map[sample_type],
			channels = n_channels,
			rate = self.rate,
			input = True,
			frames_per_buffer = self.update_window_n_frames,
			stream_callback = self.non_blocking_stream_read)


	def non_blocking_stream_read(self, in_data, frame_count, time_info, status):
		if self.verbose:
			start = time.time()

		if self.data_buffer is not None:
			self.data_buffer.append_data(np.frombuffer(in_data, dtype=np.float32))
			self.new_data = True

		if self.verbose:
			self.num_data_captures += 1
			self.data_capture_delays.append(time.time() - start)

		return in_data, pyaudio.paContinue

	def stream_start(self, data_windows_to_buffer=None):
		self.data_windows_to_buffer = data_windows_to_buffer

		if data_windows_to_buffer is None:
			self.data_windows_to_buffer = int(self.updates_per_second / 2)  # By default, buffer 0.5 second of audio
		else:
			self.data_windows_to_buffer = data_windows_to_buffer

		self.data_buffer = numpy_data_buffer(self.data_windows_to_buffer, self.update_window_n_frames)

		self.stream.start_stream()
		self.stream_start_time = time.time()

	def terminate(self):
		self.stream.stop_stream()
		self.stream.close()
		self.pa.terminate()
