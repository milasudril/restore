#!/bin/env python3

#@	{
#@	"target":{"name":"ui.wad64"},
#@	"rebuild":"always"
#@	}

import sys
import json

def compile(params):
	output_file_name = params['targets'][0]
	print(output_file_name, file=sys.stderr)
	with open(output_file_name, 'w'):
		pass

def main(argv):
	if sys.argv[1] == 'compile':
		return compile(json.loads(sys.argv[2]))

if __name__ == '__main__':
	exit(main(sys.argv))

