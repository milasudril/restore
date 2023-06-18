#!/bin/env python3

#@	{
#@	"target":{"name":"ui.wad64"},
#@	"rebuild":"always"
#@	}

import sys
import json
import os
import wad64

def strip_prefix(path, source_dir):
	return os.path.relpath(path, source_dir)

def compile(params):
	source_dir = params['build_info']['source_dir']
	output_file_name = params['targets'][0]
	files_to_add = []
	for parent_path, _, filenames in os.walk(source_dir + '/ui'):
		for f in filenames:
			files_to_add.append(os.path.join(parent_path, f))

	with wad64.Archive(output_file_name, 'rw', 'co') as archive:
		for file in files_to_add:
			archive.insert_file('cot', file, strip_prefix(file, source_dir))

def main(argv):
	if sys.argv[1] == 'compile':
		return compile(json.loads(sys.argv[2]))

if __name__ == '__main__':
	exit(main(sys.argv))

