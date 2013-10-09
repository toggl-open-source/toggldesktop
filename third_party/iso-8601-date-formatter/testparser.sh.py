#!/usr/bin/env python

outfile = file('testparser.sh', 'w')

outfile.write('#!/bin/sh\n')

hash = '#'
colon = ':'
shell_prompt = '% '
echo_format = "echo '%s'\n"
newline = '\n'
echo_by_itself = 'echo\n'

import re
bs_exp = re.compile('(\\\\*)\n')
escape_newline_exp = re.compile('\\\\\n')
empty = ''

import fileinput
holding = []
for line in fileinput.input(['testparser.sh.in']):
	if len(line) <= 1:
		#Empty line.
		continue

	if(len(bs_exp.search(line).group(1)) % 2):
		holding.append(line[:-1])
		continue
	elif holding:
		holding.append(line)
		line = '\n'.join(holding)
		del holding[:]

	line = escape_newline_exp.sub(empty, line)

	is_comment = line.startswith(hash)
	if is_comment:
		line_for_display = line[:-1].strip(hash) + colon
	else:
		line_for_display = shell_prompt + line[:-1]

	echo_line = echo_format % (line_for_display,)

	lines = [newline, echo_line]
	if is_comment:
		lines.insert(1, echo_by_itself)
	else:
		lines.append(line)
	outfile.writelines(lines)

outfile.close()

# Make it executable.
import os
os.chmod('testparser.sh', 0755)
