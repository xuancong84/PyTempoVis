import os, sys, pyaudio, threading, math, time, code
import numpy as np
from src.utils import LoopBuffer

class Stream_Reader(LoopBuffer):
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
	             buffer_fold = 0.5,
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
			input_device_index = self.device,
			input = True,
			frames_per_buffer = self.update_window_n_frames,
			stream_callback = self.non_blocking_stream_read)

		self.wav_time = None
		super().__init__(self.stream._channels, self.array_length, fold_portion=buffer_fold, dtype = self.sample_type)

	def __del__(self):
			self.stream.close()
			self.pa.terminate()

	def non_blocking_stream_read(self, in_data, frame_count, time_info, status):
		# when multiple channel, in_data is interleaved
		# print(frame_count, time_info)
		# code.interact(local=dict(globals(), **locals()) )
		new_wav = np.frombuffer(in_data, dtype=self.sample_type).reshape([frame_count,self.stream._channels]).T
		self.lock.acquire()
		try:
			self.add(new_wav, lock=False)
			self.wav_time = time_info['input_buffer_adc_time'] + frame_count / self.rate
			if self.stream_start_time is None:
				self.stream_start_time = time_info['input_buffer_adc_time']
		finally:
			self.lock.release()

		return in_data, pyaudio.paContinue

	def stream_start(self):
		self.stream_start_time = None
		self.stream.start_stream()

	def stream_stop(self):
		self.stream.stop_stream()
