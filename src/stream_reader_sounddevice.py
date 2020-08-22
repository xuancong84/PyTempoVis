import numpy as np
import time, sys, math, threading, code
import sounddevice as sd


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
	             buffer_seconds=1.0,
	             verbose = False):

		self.verbose = verbose
		self.lock = threading.Lock()
		self.sample_type = sample_type
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

		self.wav_data = None

		self.update_window_n_frames = int((self.rate / updates_per_second + 1)//2*2)

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
		self.buffer_size = int(self.rate * buffer_seconds + 0.5)
		self.info = ''
		self.new_data = False

		self.device_latency = self.device_dict[self.device]['default_low_input_latency']
		self.infos = {'overview': str(sd.query_devices()).splitlines(), 'device_list': list(sd.query_devices())}

	def __del__(self):
		self.stream.close(True)

	def non_blocking_stream_read(self, indata, frame_count, time_info, status):
		# print(indata.shape, frame_count, time_info)
		# code.interact(local=dict(globals(), **locals()) )
		self.lock.acquire()
		try:
			self.wav_data[:, :-frame_count] = self.wav_data[:, frame_count:]
			self.wav_data[:, -frame_count:] = indata.T
			self.wav_time = time_info.inputBufferAdcTime + frame_count / self.rate
			if self.stream_start_time is None:
				self.stream_start_time = time_info.inputBufferAdcTime
		finally:
			self.lock.release()

	def stream_start(self):
		self.wav_data = np.zeros([self.stream._channels, self.buffer_size], dtype=self.sample_type)
		self.wav_time = None

		self.stream.start()
		self.stream_start_time = None

	def stream_stop(self):
		self.stream.stop()
