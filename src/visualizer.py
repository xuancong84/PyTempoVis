#!/usr/bin/env python3
# coding=utf-8

import numpy as np
import time, sys, math, ctypes
# from scipy import signal
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
from src.utils import *


class TimedLevel(ctypes.Structure):
   _fields_ = [('frequency', ctypes.c_void_p*2),
               ('waveform', ctypes.c_void_p*2),
               ('timeStamp', ctypes.c_int64),
               ('state', ctypes.c_int32),
               ]

class TempoVis:
	""" The Music_Visualizer visualizes spectral FFT data """

	def __init__(self,
	             device=None,
	             rate=None,
	             updates_per_second=100,
	             tempo_buffer_seconds=20,
	             tempo_buffer_fold=0.25,
	             tempo_calc_interval=5,
	             record_latency=0,
	             n_channels=1,
	             window_title='Music Tempo Visualizer',
	             full_screen=False,
	             verbose=False):

		# Create audio stream reader
		try:
			from src.stream_reader_sounddevice import Stream_Reader
			self.stream_reader = Stream_Reader(
				device = device,
				rate = rate,
				n_channels = n_channels,
				buffer_seconds = tempo_buffer_seconds,
				buffer_fold = tempo_buffer_fold,
				updates_per_second = updates_per_second,
				verbose = verbose)
		except:
			try:
				from src.stream_reader_pyaudio import Stream_Reader
				self.stream_reader = Stream_Reader(
					device = device,
					rate = rate,
					n_channels = n_channels,
					buffer_seconds = tempo_buffer_seconds,
					buffer_fold = tempo_buffer_fold,
					updates_per_second = updates_per_second,
					verbose = verbose)
			except:
				raise Exception('device init failed, neither pyaudio nor sounddevice is working')

		self.rate = int(self.stream_reader.rate)
		self.verbose = verbose

		# Control parameters
		self.isFullScreen = full_screen
		self.windowTitle = window_title
		self.FFT_size = 1024
		self.last_tempo_calc_time = None
		self.tempo_calc_interval = tempo_calc_interval

		# Initialize OpenGL window
		glutInit()
		glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH)
		glutInitWindowSize(1024, 576)
		self.hWindow = glutCreateWindow(self.windowTitle)
		glutDisplayFunc(self.drawFunc)
		glutIdleFunc(self.drawFunc)
		glutKeyboardFunc(self.keyFunc)
		glutSpecialFunc(self.keyFunc)
		glutMouseFunc(self.mouseClickFunc)
		glutMotionFunc(self.mouseMoveFunc)

		# Load and initialize DLL TempoVis class
		self.tempoVis = ctypes.CDLL(os.getcwd()+'/tempovis.so')
		self.tempoVis.CreateTempoVis()
		self.gm_debug = ctypes.c_byte.in_dll(self.tempoVis, 'gm_debug')
		self.gm_showFPS = ctypes.c_byte.in_dll(self.tempoVis, 'gm_showFPS')
		self.gm_fullScreen = ctypes.c_byte.in_dll(self.tempoVis, 'gm_fullScreen')
		self.RecordLatency = ctypes.c_float.in_dll(self.tempoVis, 'RecordLatency')

		# Set global variables in dynamic library
		self.gm_debug.value = 1
		self.gm_showFPS.value = 1
		self.record_latency = int(record_latency*100+0.5)
		self.RecordLatency.value = self.record_latency/100.
		if self.isFullScreen:
			glutFullScreen()
			self.gm_fullScreen.value = 1

	def mouseMoveFunc(self, x, y):
		self.tempoVis.onMouseMove(x, y)

	def mouseClickFunc(self, button, state, x, y):
		if button == GLUT_LEFT_BUTTON:
			if state == GLUT_DOWN:
				self.tempoVis.onMouseButton(1, x, y)
			else:
				self.tempoVis.onMouseButton(0, x, y)
		elif button == GLUT_MIDDLE_BUTTON:
			if state == GLUT_DOWN:
				self.tempoVis.onMouseButton(3, x, y)
			elif state == GLUT_UP:
				self.tempoVis.onMouseButton(2, x, y)

	def keyFunc(self, key, x, y):
		if key == b'\x1b':
			glutLeaveMainLoop()
		elif key == GLUT_KEY_F1:
			self.gm_debug.value = min(3, self.gm_debug.value+1)
		elif key == GLUT_KEY_F2:
			self.gm_debug.value = max(0, self.gm_debug.value-1)
		elif key in [b'+', b'=']:
			self.record_latency += 1
			self.RecordLatency.value = self.record_latency / 100.
		elif key in [b'-', b'_']:
			self.record_latency -= 1
			self.RecordLatency.value = self.record_latency / 100.

	def drawFunc(self):
		# return self.drawFunc1()

		length = self.FFT_size*2+1
		wav, tms = self.stream_reader.get(length=length), self.stream_reader.wav_time
		if tms is None or wav.shape[1]<length: return

		# Pre-emphasis
		wav = wav[:, :-1] - wav[:, 1:]*0

		# Compute FFT
		fft = np.abs(np.fft.rfft(wav)[:, :self.FFT_size], dtype=wav.dtype)
		# fft = np.log1p(fft)
		stereo = wav.shape[0]>1
		tml = TimedLevel((ctypes.c_void_p*2)(fft[0,:].ctypes.data,
		                                     fft[1,:].ctypes.data if stereo else 0),
		                (ctypes.c_void_p*2)(wav[0,:].ctypes.data,
		                                    wav[1,:].ctypes.data if stereo else 0),
		                ctypes.c_int64(int(tms*1e7)),
		                ctypes.c_int32(2))
		self.tempoVis.DrawFrame(ctypes.pointer(tml))
		glutSwapBuffers()

		# wav0 = self.stream_reader.get()
		# Compute tempo at regular intervals
		if self.last_tempo_calc_time is None:
			self.last_tempo_calc_time = tms
		elif tms-self.last_tempo_calc_time >= self.tempo_calc_interval:
			self.last_tempo_calc_time = tms
			wav = self.stream_reader.get()
			if False:
				import pyaudio
				p = pyaudio.PyAudio()
				stream = p.open(format = pyaudio.paFloat32, channels = 2, rate = self.rate, output = True)
				stream.write(wav.tobytes())
			if wav.shape[0]>1: wav = wav.mean(axis=0, keepdims=True)
			print('compute tempo: length=%s sec'%(wav.shape[1]/self.stream_reader.rate))
			self.tempoVis.CreateTempoThread(ctypes.c_void_p(wav[0,:].ctypes.data),
			                                ctypes.c_int32(wav.shape[1]),
			                                ctypes.c_int32(int(self.stream_reader.rate)))


	def drawFunc1(self):
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
		glMatrixMode(GL_MODELVIEW)
		glLoadIdentity()

		glMatrixMode(GL_PROJECTION)
		glLoadIdentity()
		vp = glGetIntegerv(GL_VIEWPORT) # [x, y, width, height]
		glOrtho(vp[0], vp[2], vp[1], vp[3], 0.0, 1.0)
		X, Y, Width, Height = vp

		glColor3f(1.0, 0.0, 3.0)

		theta = time.time()%(np.pi*2)
		dx = 100 * np.cos(theta) + vp[2]/2
		dy = 100 * np.sin(theta) + vp[3]/2

		glBegin(GL_QUADS)
		glVertex2f(-100+dx, 100+dy)
		glVertex2f(100+dx, 100+dy)
		glVertex2f(100+dx, -100+dy)
		glVertex2f(-100+dx, -100+dy)
		glEnd()

		if True:
			# draw waveform 2 channel
			glEnableClientState(GL_VERTEX_ARRAY)
			glColor3f(1, 1, 1)
			width = vp[2]

			if self.stream_reader.wav_data.shape[0] == 2:
				glLoadIdentity()
				glOrtho(vp[0], vp[2], -3, 1, 0.0, 1.0)
				buf = np.stack([np.arange(width), self.stream_reader.wav_data[0, -width:]], axis=1).flatten()
				glVertexPointer(2, GL_FLOAT, 0, buf)
				glDrawArrays(GL_LINE_STRIP, 0, width)

				glLoadIdentity()
				glOrtho(vp[0], vp[2], -1, 3, 0.0, 1.0)
				buf = np.stack([np.arange(width), self.stream_reader.wav_data[1, -width:]], axis=1).flatten()
				glVertexPointer(2, GL_FLOAT, 0, buf)
				glDrawArrays(GL_LINE_STRIP, 0, width)
			else:
				glLoadIdentity()
				glOrtho(vp[0], vp[2], -1, 1, 0.0, 1.0)
				buf = np.stack([np.arange(width), self.stream_reader.wav_data[0, -width:]], axis=1).flatten()
				glVertexPointer(2, GL_FLOAT, 0, buf)
				glDrawArrays(GL_LINE_STRIP, 0, width)

			glDisableClientState(GL_VERTEX_ARRAY)

			# draw text
			glLoadIdentity()
			glOrtho(0, Width, Height - 1, 0, -1, 1)
			for ii,font in enumerate([
			    GLUT_BITMAP_9_BY_15,
			    GLUT_BITMAP_8_BY_13,
			    GLUT_BITMAP_TIMES_ROMAN_10,
			    GLUT_BITMAP_TIMES_ROMAN_24,
			    GLUT_BITMAP_HELVETICA_10,
			    GLUT_BITMAP_HELVETICA_12,
			    GLUT_BITMAP_HELVETICA_18
			]):
				glRasterPos2i(10, 10+ii*20)
				st = 100
				for c in range(st, 256):
					glutBitmapCharacter(font, (ctypes.c_int)(c))

		glutSwapBuffers()

	def start(self):
		# Start audio recording
		self.stream_reader.stream_start()

		# Enter GLUT mainloop never returning
		glutMainLoop()
