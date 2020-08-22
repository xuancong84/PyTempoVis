import numpy as np
import time, math, scipy, os, sys
from collections import deque
from scipy.signal import savgol_filter

from src.fft import getFFT
from src.visualizer import *

class TempoVis:
	"""
	The Audio_Analyzer class provides access to continuously recorded
	(and mathematically processed) audio data.

	Arguments:

		device: int or None:      Select which audio stream to read .
		rate: float or None:      Sample rate to use. Defaults to something supported.
		FFT_window_size_ms: int:  Time window size (in ms) to use for the FFT transform
		updatesPerSecond: int:    How often to record new data.

	"""

	def __init__(self,
	             device=None,
	             rate=None,
	             updates_per_second=100,
	             n_channels=1,
	             verbose=False):

		self.rate = rate
		self.verbose = verbose

		# from src.stream_reader_pyaudio import Stream_Reader
		# self.stream_reader = Stream_Reader(
		# 	device=device,
		# 	rate=rate,
		# 	n_channels=n_channels,
		# 	updates_per_second=updates_per_second,
		# 	verbose=verbose)

		try:
			from src.stream_reader_pyaudio import Stream_Reader
			self.stream_reader = Stream_Reader(
				device=device,
				rate=rate,
				n_channels=n_channels,
				updates_per_second=updates_per_second,
				verbose=verbose)
		except:
			try:
				from src.stream_reader_sounddevice import Stream_Reader
				self.stream_reader = Stream_Reader(
					device=device,
					rate=rate,
					n_channels=n_channels,
					updates_per_second=updates_per_second,
					verbose=verbose)
			except:
				raise Exception('device init failed, neither pyaudio nor sounddevice is working')

		self.rate = self.stream_reader.rate
		self.visualizer = Music_Visualizer(self.stream_reader)

	def start(self):
		self.stream_reader.stream_start()
		self.visualizer.start()
