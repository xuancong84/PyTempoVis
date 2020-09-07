#!/usr/bin/env python3
# coding=utf-8

import numpy as np
import time, sys, math, ctypes
# from scipy import signal
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *


class TimedLevel(ctypes.Structure):
   _fields_ = [('frequency', ctypes.c_void_p*2),
               ('waveform', ctypes.c_void_p*2),
               ('timeStamp', ctypes.c_int64),
               ('state', ctypes.c_int32),
               ]

class Music_Visualizer:
	""" The Music_Visualizer visualizes spectral FFT data """

	def __init__(self, stream_reader, title='OpenGL window', full_screen=False):
		# Control parameters
		self.isFullScreen = full_screen
		self.windowTitle = title
		self.FFT_size = 1024
		self.stream_reader = stream_reader  # imported audio stream reader object, must have .wav_data & .wav_time

		# Initialize OpenGL window
		glutInit()
		glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH)
		glutInitWindowSize(1024, 576)
		self.hWindow = glutCreateWindow(self.windowTitle)
		if self.isFullScreen:
			glutFullScreen()
		glutDisplayFunc(self.drawFunc)
		glutIdleFunc(self.drawFunc)

		# Load and initialize DLL TempoVis class
		self.tempoVis = ctypes.CDLL(os.getcwd()+'/tempovis.so')
		self.tempoVis.CreateTempoVis()
		ctypes.c_byte.in_dll(self.tempoVis, 'gm_debug').value = 1
		ctypes.c_byte.in_dll(self.tempoVis, 'gm_showFPS').value = 1


	def drawFunc(self):
		# return self.drawFunc1()

		self.stream_reader.lock.acquire()
		wav, tms = self.stream_reader.wav_data[0, -self.FFT_size*2-1:].copy()*128, self.stream_reader.wav_time
		self.stream_reader.lock.release()

		if tms is None: return
		# Pre-emphasis
		wav = wav[:-1] - wav[1:]*0.5

		# Compute FFT
		# fft = np.log1p(np.abs(np.fft.rfft(wav, norm="ortho")[:self.FFT_size]))*4096
		fft = np.abs(np.fft.rfft(wav)[:self.FFT_size])*8
		tml = TimedLevel((ctypes.c_void_p*2)(fft.astype(np.float32).ctypes.data, 0),
		                 (ctypes.c_void_p*2)(wav.ctypes.data, 0),
		                 int((tms-self.stream_reader.stream_start_time)*1e7), 2)
		self.tempoVis.DrawFrame(ctypes.pointer(tml))
		glutSwapBuffers()


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
		# Enter GLUT mainloop never returning
		glutMainLoop()
