#!/usr/bin/env python3
# coding=utf-8

import os, sys, argparse, time, json
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *

if False:
	w, h = 500, 500


	def square():
		glBegin(GL_QUADS)
		glVertex2f(100, 100)
		glVertex2f(200, 100)
		glVertex2f(200, 200)
		glVertex2f(100, 200)
		glEnd()


	def iterate():
		glViewport(0, 0, 500, 500)
		glMatrixMode(GL_PROJECTION)
		glLoadIdentity()
		glOrtho(0.0, 500, 0.0, 500, 0.0, 1.0)
		glMatrixMode(GL_MODELVIEW)
		glLoadIdentity()


	def foo():
		a = 10
		code.interact(local=locals())
		return a


	t0 = time.time()
	N = 0


	def showScreen():
		global N
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
		glLoadIdentity()
		iterate()
		glColor3f(1.0, 0.0, 3.0)
		square()
		glutSwapBuffers()
		N += 1
		print('FPS=%f' % (N / (time.time() - t0)), end='\r')


	glutInit()
	glutInitDisplayMode(GLUT_RGBA)
	glutInitWindowSize(500, 500)
	wind = glutCreateWindow("OpenGL Coding Practice")
	# glutFullScreen()
	glutDisplayFunc(showScreen)
	glutIdleFunc(foo)
	glutMainLoop()
	sys.exit(0)

if __name__ == '__main__':
	parser = argparse.ArgumentParser(usage='$0 [options]', description='Music Tempo Visualization',
	                                 formatter_class=argparse.ArgumentDefaultsHelpFormatter)
	parser.add_argument('--fps', '-fps', help='update rate in frames-per-second', type=float, default=60)
	parser.add_argument('--device', '-d', help='device index', type=int, default=None)
	parser.add_argument('--list-devices', '-ls', help='list recording devices and quit', action='store_true')
	parser.add_argument('--sample-rate', '-sr', help='audio recording sampling rate', type=int, default=44100)
	parser.add_argument('--fullscreen', '-fs', help='draw full screen', action='store_true')
	parser.add_argument('--verbose', '-v', help='verbose mode', action='store_true')
	# nargs='?': optional positional argument; action='append': multiple instances of the arg; type=; default=
	opt = parser.parse_args()
	globals().update(vars(opt))

	from src.stream_analyzer import *

	tv = TempoVis(
		device = device,  # Manually play with this (int) if you don't see anything
		rate = sample_rate,  # Audio samplerate, None uses the default source settings
		FFT_window_size_ms = 60,  # Window size used for the FFT transform
		updates_per_second = fps,  # How often to read the audio stream for new data
		smoothing_length_ms = 50,  # Apply some temporal smoothing to reduce noisy features
		n_frequency_bins = 400,  # The FFT features are grouped in bins
		visualize = 1,  # Visualize the FFT features with PyGame
		verbose = verbose  # Print running statistics (latency, fps, ...)
	)

	if list_devices:
		print(json.dumps(tv.stream_reader.infos, indent=1))
		sys.exit(0)

	tv.start()
	last_update = time.time()
	while True:
		if (time.time() - last_update) > (1. / fps):
			last_update = time.time()
			raw_fftx, raw_fft, binned_fftx, binned_fft = ear.get_audio_features()
