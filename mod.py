#!/usr/bin/env python3
# coding=utf-8

import os,sys,argparse,re

if __name__=='__main__':
	parser = argparse.ArgumentParser(usage='$0 arg1 1>output 2>progress', description='what this program does',
			formatter_class=argparse.ArgumentDefaultsHelpFormatter)
	parser.add_argument('-optional', help='optional argument')
	#nargs='?': optional positional argument; action='append': multiple instances of the arg; type=; default=
	opt=parser.parse_args()
	globals().update(vars(opt))

	inASM = False
	while True:
		try:
			L = input()
		except:
			break

		# if L.strip()==')':
		# 	print('%s;'%L)
		# else:
		# 	print(L)

		if L.strip().startswith('asm('):
			inASM = True
			print(L)
			continue

		if inASM:
			if L.strip()=='':
				print()
			elif L.strip()==')':
				inASM = False
				print(L+';')
			else:
				n_indent = len(L)-len(L.lstrip())
				print('%s"%s\\n"'%(L[:n_indent], L.strip()))
		else:
			print(L)
