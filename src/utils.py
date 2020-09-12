import threading
import numpy as np


# can allocate large size and high performance
class LoopBuffer:
	def __init__(self, width, length, fold_portion=0.5, dtype=np.float32):
		self.width = width
		self.length = int(length*(1+fold_portion)+0.5)
		self.data = np.zeros([width, self.length], dtype=dtype)
		self.currHead = 0  # pointer to current end-position
		self.currTotalPosi = 0  # current global position
		self.fold_size = self.length - length  # proportion to fold back when buffer is full
		self.lock = threading.Lock()

	def add(self, data, lock=True):
		if lock:
			self.lock.acquire()
			try:
				W, L = data.shape
				if self.currHead + L > self.length:
					self.data[:, 0:-self.fold_size] = self.data[:, self.fold_size:]
					self.currHead -= self.fold_size
				self.data[:, self.currHead:self.currHead + L] = data
				self.currHead += L
				self.currTotalPosi += L
			finally:
				self.lock.release()
		else:
			W, L = data.shape
			if self.currHead + L > self.length:
				self.data[:, 0:-self.fold_size] = self.data[:, self.fold_size:]
				self.currHead -= self.fold_size
			self.data[:, self.currHead:self.currHead + L] = data
			self.currHead += L
			self.currTotalPosi += L

	def get(self, length=None, clone=True):
		if clone:
			self.lock.acquire()
			try:
				return (self.data[:, (self.currHead-length if length else 0):self.currHead]).copy()
			finally:
				self.lock.release()

		# returning direct buffer requires caller to acquire lock
		return self.data[:, (self.currHead-length if length else 0):self.currHead]


def playsound(wav, sr=44100):
	# Note: multi-channel audio are in [n_channels, n_samples] format
	if not isinstance(wav, np.ndarray):
		wav = np.array(wav)

	if wav.dtype != np.float32:
		wav = wav.astype(np.float32)

	if len(wav.shape) == 1:
		wav = wav.reshape([-1, 1])
	elif wav.shape[1]>wav.shape[0]:
		wav = wav.T

	import pyaudio
	p = pyaudio.PyAudio()
	stream = p.open(format=pyaudio.paFloat32,
	                channels=wav.shape[-1],
	                rate=sr,
	                output=True)

	try:
		stream.write(wav.tobytes())
	finally:
		stream.close()
		p.terminate()