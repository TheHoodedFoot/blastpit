#!/usr/bin/env python3

import re
import subprocess

pattern = re.compile(r'\"directory\": \"(.*)\",.*file\": \"(.*)\"', re.DOTALL)

prev_line = ""

for i, line in enumerate(open('compile_commands.json')):
    found = 0
    lines = prev_line + line
    for match in re.finditer(pattern, lines ):
        rel_filename = str(match.groups()[0]) + '/' + str(match.groups()[1])
        abs_filename = subprocess.check_output(['readlink', '-f', rel_filename]).decode('utf-8').rstrip()

        print('        \"file\": \"' + abs_filename + '\"')
        found = 1
    if found == 0:
        print(line.rstrip())
    prev_line = line

