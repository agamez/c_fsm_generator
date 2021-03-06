#!/usr/bin/python3
"""
Usage: gen_fsm -h
       gen_fsm -N <fsm_name> -I <orig_path> -O <dest_path> [-T <templates_prefix>]

Options:
  -I <orig_path>				Origin path from which to read CSV files
  -O <dest_path>				Destination path for generated files
  -N <fsm_name>					FSM name and prefix of C objects
  -T <templates_prefix>				Location of templates and files
"""

from docopt import docopt
import jinja2
import csv
from inspect import getsourcefile
import os
import shutil

if __name__ == "__main__":
	args = docopt(__doc__)
	input_prefix = args['-I'] + '/' + args['-N']
	output_prefix = args['-O'] + '/' + args['-N']
	if args['-T']:
		gen_fsm_path = args['-T'] + '/'
	else:
		gen_fsm_path = os.path.dirname(getsourcefile(lambda:0))
	os.makedirs(args['-O'], exist_ok = True)

	# Read information
	states = list()
	with open(input_prefix + '_states.csv') as csvfile:
		states_reader = csv.reader(csvfile, skipinitialspace = True)
		for row in states_reader:
			states.append(row[0])

	events = list()
	with open(input_prefix + '_events.csv') as csvfile:
		events_reader = csv.DictReader(csvfile, skipinitialspace = True)
		for row in events_reader:
			events.append(row)

	transitions = dict()
	with open(input_prefix + '_transitions.csv') as csvfile:
		transitions_reader = csv.DictReader(csvfile, skipinitialspace = True)
		for row in transitions_reader:
			if row['State'] in transitions:
				transitions[row['State']].append(row)
			else:
				transitions[row['State']] = [row, ]

	# Create templates environment
	ji2 = jinja2.Environment(loader = jinja2.FileSystemLoader(gen_fsm_path), trim_blocks = True, keep_trailing_newline = True, lstrip_blocks = True)

	# Generate outputfiles
	fsm_h_template = ji2.get_template('fsm.h.j2')
	with open(output_prefix + '_fsm.h', 'w') as fd:
		fd.write(fsm_h_template.render(states = states, events = events, transitions = transitions, PREFIX = args['-N']))

	graphviz_template = ji2.get_template('graph.dot.j2')
	with open(output_prefix + '_transitions.dot', 'w') as fd:
		fd.write(graphviz_template.render(states = states, events = events, transitions = transitions, PREFIX = args['-N']))

	states_template = ji2.get_template('states.c.j2')
	with open(output_prefix + '_states.c', 'w') as fd:
		fd.write(states_template.render(states = states, events = events, transitions = transitions, PREFIX = args['-N']))

	# Copy non template based files
	shutil.copyfile(gen_fsm_path + '/fsm.h', args['-O'] + '/fsm.h')
	shutil.copyfile(gen_fsm_path + '/fsm.c', args['-O'] + '/fsm.c')
