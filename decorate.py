#!/usr/bin/python

import os
import re


def handleFile(path):
	print path
	api_h_path = ("../" * (path.count("/") - 2)) + "api.h"
	lines = file(path).readlines()
	lineNumber = 0
	firstInclude = None
	api_h_included = False
	for l in lines:
		# include statement?
		m = re.match(r'#include\s*["<]([\-_a-zA-Z0-9\.\\/]+)[">]\s*', l)
		if m:
			included_file = m.groups(1)[0]
			if firstInclude == None:
				firstInclude = lineNumber
			print "\t includes", included_file
			if not api_h_included: api_h_included = "api.h" in included_file
		
		# forward declaration?
		m = re.match(r'\s*class\s+([_a-zA-Z0-9]+)\s*;\s*$', l)
		forwardDecl = True
		if m:
			forwardDecl = True
			print "\t forward declaration: ", m.groups(1)[0]
		else:	
			# class definition
			#m = re.match(r'(\s*)class(\s+([\S]+))?\s+([_a-zA-Z0-9]+)([ ;]*?)$', l)
			m = re.match(r'(\s*)class\s+(.*)$', l)
			if m:
				rest = m.groups(1)[1]
				decorated = "AVG_API" in rest
				print "\t\tfound class definition",rest, " AVG_API:", decorated
			
				if not decorated:
					lines[lineNumber] = m.groups(1)[0] + "class AVG_API "+ rest + "\n"
				
		
		lineNumber+=1
		
	print "\tincludes api.h: ", api_h_included
	if firstInclude:
		print "\tfirst #include at line: ", firstInclude +1
	
	if not api_h_included and firstInclude:
		lines.insert(firstInclude, '#include "'+api_h_path+'"\n')
	
	content = "".join(lines)
	file(path, "w").write(content)
	
headers = os.popen('find .|grep "\.h$"').readlines()
headers = map(lambda x: x.strip(), headers)

for f in headers:
	handleFile(f)
#handleFile("./src/video/AudioDecoderThread.h")
