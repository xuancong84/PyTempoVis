import numpy as np
import time, sys, math, threading, code
import sounddevice as sd
from src.utils import LoopBuffer

sys.g_N=0

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
			device=self.device,
			channels=n_channels,
			dtype=sample_type,
			latency='low',
			extra_settings=None,
			callback=self.non_blocking_stream_read)

		self.rate = self.stream.samplerate
		self.device = self.stream.device

		self.updates_per_second = self.rate / self.update_window_n_frames
		self.array_length = int(self.rate * buffer_seconds + 0.5)
		self.info = ''
		self.new_data = False

		self.device_latency = self.device_dict[self.device]['default_low_input_latency']
		self.infos = {'overview': str(sd.query_devices()).splitlines(), 'device_list': list(sd.query_devices())}

		self.wav_time = None    # always starts from zero
		super().__init__(self.stream._channels, self.array_length, fold_portion=buffer_fold, dtype=self.sample_type)

	def __del__(self):
		self.stream.close(True)

	def non_blocking_stream_read(self, indata, frame_count, time_info, status):
		# print(indata.shape, frame_count, end='\r')
		# code.interact(local=dict(globals(), **locals()) )
		self.lock.acquire()
		try:
			self.add(indata.T, lock=False)
			self.wav_time = self.currTotalPosi / self.rate
		finally:
			self.lock.release()

	def stream_start(self):
		self.stream_start_time = time.time()
		self.stream.start()

	def stream_stop(self):
		self.stream.stop()
