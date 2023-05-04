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
	states_functions = set()
	with open(input_prefix + '_states.csv') as csvfile:
		states_reader = csv.DictReader(csvfile, skipinitialspace = True)
		for row in states_reader:
			states.append(row)
			if (row['Enter_Function'] != "NULL"):
				states_functions.add(row['Enter_Function'])
			if (row['Exit_Function'] != "NULL"):
				states_functions.add(row['Exit_Function'])

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

	processing = dict()
	processing_functions = set()
	def add_processing_function(state, row):
		if state in processing:
			processing[state].append(row)
		else:
			processing[state] = [row, ]
		processing_functions.add(row['Process_Function'])

	with open(input_prefix + '_processing.csv') as csvfile:
		processing_reader = csv.DictReader(csvfile, skipinitialspace = True)
		for row in processing_reader:
			state = row.pop('State')
			if state == '*':
				for state in states:
					add_processing_function(state['State'], row)
			else:
				add_processing_function(state, row)
			processing_functions.add(row['Process_Function'])

	# Create templates environment
	ji2 = jinja2.Environment(loader = jinja2.FileSystemLoader(gen_fsm_path), trim_blocks = True, keep_trailing_newline = True, lstrip_blocks = True)

	# Generate outputfiles
	fsm_h_template = ji2.get_template('fsm.h.j2')
	with open(output_prefix + '_fsm.h', 'w') as fd:
		fd.write(fsm_h_template.render(states = states, states_functions = states_functions, events = events, transitions = transitions, processing = processing, processing_functions = processing_functions, PREFIX = args['-N']))

	graphviz_template = ji2.get_template('graph.dot.j2')
	with open(output_prefix + '_transitions.dot', 'w') as fd:
		fd.write(graphviz_template.render(states = states, states_functions = states_functions, events = events, transitions = transitions, processing = processing, processing_functions = processing_functions, PREFIX = args['-N']))

	# Copy non template based files
	shutil.copyfile(gen_fsm_path + '/fsm.h', args['-O'] + '/fsm.h')
	shutil.copyfile(gen_fsm_path + '/fsm.c', args['-O'] + '/fsm.c')
	shutil.copyfile(gen_fsm_path + '/freebsd-queue.h', args['-O'] + '/freebsd-queue.h')
