#!/usr/bin/env python3
"""
Usage: gen_fsm -h
       gen_fsm -N <fsm_name>

Options:
  -N <fsm_name>					FSM name and prefix of C objects
"""

from docopt import docopt
import jinja2
import csv
from os import getcwd

if __name__ == "__main__":
	args = docopt(__doc__)

	ji2 = jinja2.Environment(loader = jinja2.FileSystemLoader(getcwd()), trim_blocks = True, keep_trailing_newline = True, lstrip_blocks = True)
	states_h_template = ji2.get_template('states.h.j2')

	states = list()
	with open('states.csv') as csvfile:
		states_reader = csv.reader(csvfile, skipinitialspace = True)
		for row in states_reader:
			states.append(row[0])
	with open(args['-N'] + '_fsm_states.h', 'w') as fd:
		fd.write(states_h_template.render(states = states, PREFIX = args['-N']))
