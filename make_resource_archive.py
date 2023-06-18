#!/bin/env python3

#@	{
#@	"target":{"name":"ui.wad64"},
#@	"rebuild":"always"
#@	}

import sys
import json
import os
import wad64
import mimetypes

def strip_prefix(path, source_dir):
	return os.path.relpath(path, source_dir)

def get_file_info(src_file, source_dir):
	file = dict()
	file['name'] = strip_prefix(src_file, source_dir)
	file['mime_type'] = mimetypes.guess_type(src_file)[0]
	file['last_modified'] = os.stat(src_file).st_mtime
	return file

def compile(params):
	source_dir = params['build_info']['source_dir']
	output_file_name = params['targets'][0]
	src_files = []
	dest_files = []

	for parent_path, _, filenames in os.walk(source_dir + '/ui'):
		for f in filenames:
			src_file = os.path.join(parent_path, f)
			src_files.append(src_file)
			dest_files.append(get_file_info(src_file, source_dir))

	with wad64.Archive(output_file_name, 'rw', 'co') as archive:
		for file in zip(src_files, dest_files):
			archive.insert_file('cot', file[0], file[1]['name'])
		metadata = json.dumps(dest_files)
		print(metadata, file=sys.stderr)

def main(argv):
	if sys.argv[1] == 'compile':
		return compile(json.loads(sys.argv[2]))

if __name__ == '__main__':
	exit(main(sys.argv))

