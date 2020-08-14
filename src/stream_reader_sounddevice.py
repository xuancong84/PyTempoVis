import numpy as np
import time, sys, math
from collections import deque
import sounddevice as sd

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
		self.device_dict = sd.query_devices()

		try:
			sd.check_input_settings(device=device, channels=n_channels, dtype=sample_type, extra_settings=None, samplerate=rate)
		except:
			rate = device = n_channels = sample_type = None

		self.rate = rate
		if rate is not None:
			sd.default.samplerate = rate

		self.device = device
		if device is not None:
			sd.default.device = device

		self.data_buffer = None

		# This part is a bit hacky, need better solution for this:
		# Determine what the optimal buffer shape is by streaming some test audio
		self.optimal_data_lengths = [updates_per_second]
		with sd.InputStream(samplerate=self.rate,
		                    blocksize=0,
		                    device=self.device,
		                    channels=n_channels,
		                    dtype=sample_type,
		                    latency='low',
		                    callback=self.test_stream_read):
			time.sleep(0.2)

		self.update_window_n_frames = max(self.optimal_data_lengths)
		del self.optimal_data_lengths

		self.stream = sd.InputStream(
			samplerate=self.rate,
			blocksize=self.update_window_n_frames,
			device=None,
			channels=n_channels,
			dtype=sample_type,
			latency='low',
			extra_settings=None,
			callback=self.non_blocking_stream_read)

		self.rate = self.stream.samplerate
		self.device = self.stream.device

		self.updates_per_second = self.rate / self.update_window_n_frames
		self.info = ''
		self.new_data = False

		if self.verbose:
			self.data_capture_delays = deque(maxlen=32)
			self.num_data_captures = 0

		self.device_latency = self.device_dict[self.device]['default_low_input_latency']
		self.infos = {'overview': str(sd.query_devices()).splitlines(), 'device_list': list(sd.query_devices())}


	def non_blocking_stream_read(self, indata, frames, time_info, status):
		start = time.time()
		self.status = status

		if self.data_buffer is not None:
			self.data_buffer.append_data(indata[:, 0])
			self.new_data = True

		if self.verbose:
			self.num_data_captures += 1
			self.data_capture_delays.append(time.time() - start)

		return

	def test_stream_read(self, indata, frames, time_info, status):
		'''
		Dummy function to determine what blocksize the stream is using
		'''
		self.optimal_data_lengths.append(len(indata[:, 0]))
		return

	def stream_start(self, data_windows_to_buffer=None):
		self.data_windows_to_buffer = data_windows_to_buffer

		if data_windows_to_buffer is None:
			self.data_windows_to_buffer = int(self.updates_per_second / 2)  # By default, buffer 0.5 second of audio
		else:
			self.data_windows_to_buffer = data_windows_to_buffer

		self.data_buffer = numpy_data_buffer(self.data_windows_to_buffer, self.update_window_n_frames)

		self.stream.start()
		self.stream_start_time = time.time()

	def terminate(self):
		self.stream.stop()
