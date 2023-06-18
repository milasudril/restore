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

def get_file_info(src_file):
	file = dict()
	file['mime_type'] = mimetypes.guess_type(src_file)[0]
	file['last_modified'] = os.stat(src_file).st_mtime
	return file

def remove_old_files(archive, new_files):
	existing_files = set(archive.ls().keys())
	for file in existing_files.difference(new_files):
		archive.wipe_file(file)

def get_file_metadata(archive):
	# NOTE: This solution is fine for a build system, but is not safe in case the file was removed
	#       between check and read
	if 'file_metadata.json' in archive.ls():
		return json.loads(archive.extract_data('file_metadata.json').decode('utf-8'))
	return dict()

def compile(params):
	source_dir = params['build_info']['source_dir']
	output_file_name = params['targets'][0]
	src_files = []
	dest_files = dict()

	for parent_path, _, filenames in os.walk(source_dir + '/ui'):
		for f in filenames:
			src_file = os.path.join(parent_path, f)
			src_files.append(src_file)
			dest_file = strip_prefix(src_file, source_dir)
			dest_files[dest_file] = get_file_info(src_file)

	with wad64.Archive(output_file_name, 'rw', 'co') as archive:
		file_metadata = get_file_metadata(archive)

		# NOTE: since metadata is not in the source directory, it will be removed from the archive
		#       at this point. Thus, we use `ct` when adding it back.
		remove_old_files(archive, set(dest_files.keys()))

		existing_files = archive.ls()
		for file in zip(src_files, dest_files.items()):
			dest_file = file[1][0]
			if dest_file in existing_files and dest_file in file_metadata:
				if file[1][1]['last_modified'] > file_metadata[dest_file]['last_modified']:
					archive.wipe_file(dest_file)
					archive.insert_file('ct', file[0], file[1][0])
			else:
				archive.insert_file('ct', file[0], file[1][0])

		archive.insert_data('ct', bytes(json.dumps(dest_files), 'utf-8'), 'file_metadata.json')

def main(argv):
	if sys.argv[1] == 'compile':
		return compile(json.loads(sys.argv[2]))

if __name__ == '__main__':
	exit(main(sys.argv))
