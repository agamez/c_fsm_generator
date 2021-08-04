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
from inspect import getsourcefile
import os

if __name__ == "__main__":
	args = docopt(__doc__)

	gen_fsm_path = os.path.dirname(getsourcefile(lambda:0))

	ji2 = jinja2.Environment(loader = jinja2.FileSystemLoader(gen_fsm_path), trim_blocks = True, keep_trailing_newline = True, lstrip_blocks = True)
	fsm_h_template = ji2.get_template('fsm.h.j2')

	states = list()
	with open(args['-N'] + '_states.csv') as csvfile:
		states_reader = csv.reader(csvfile, skipinitialspace = True)
		for row in states_reader:
			states.append(row[0])

	events = list()
	with open(args['-N'] + '_events.csv') as csvfile:
		events_reader = csv.DictReader(csvfile, skipinitialspace = True)
		for row in events_reader:
			events.append(row)

	transitions = dict()
	with open(args['-N'] + '_transitions.csv') as csvfile:
		transitions_reader = csv.DictReader(csvfile, skipinitialspace = True)
		for row in transitions_reader:
			if row['State'] in transitions:
				transitions[row['State']].append(row)
			else:
				transitions[row['State']] = [row, ]

	with open(args['-N'] + '_fsm.h', 'w') as fd:
		fd.write(fsm_h_template.render(states = states, events = events, transitions = transitions, PREFIX = args['-N']))

	graphviz_template = ji2.get_template('graph.dot.j2')
	with open(args['-N'] + '_transitions.dot', 'w') as fd:
		fd.write(graphviz_template.render(states = states, events = events, transitions = transitions, PREFIX = args['-N']))
