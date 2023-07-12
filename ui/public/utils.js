"use strict";

let strcmp = new Intl.Collator(undefined, {numeric:true, sensitivity: "base"}).compare;

function find_if(array, predicate)
{
	let ret = array.findIndex(predicate);
	return ret === -1? array.length : ret;
}

function find(array, value)
{
	let ret = array.indexOf(value);
	return ret === -1? array.length : ret;
}

function append_blobs_to_string(str, blobs)
{
	let encoded_string = new TextEncoder().encode(str);
	if(Object.entries(blobs).length === 0)
	{ return encoded_string; }

	let buffer_size = encoded_string.length + 1;
	for(let key in blobs)
	{ buffer_size += blobs[key].byteLength; }

	let ret = new Uint8Array(buffer_size);
	ret.set(encoded_string, 0);

	let current_offset = encoded_string.length + 1;
	for(let key in blobs)
	{
		let blob = blobs[key];
		ret.set(new Uint8Array(blob), current_offset);
		current_offset += blob.byteLength;
	}

	return ret;
}

function make_request_body(fields, blobs)
{
	let message = {
		fields: fields,
		blobs: {}
	};

	let current_offset = 1;
	for(let key in blobs)
	{
		let blob = blobs[key];
		let blob_size = blob.byteLength;
		let blob_info = {
			size: blob_size,
			start_offset: current_offset
		};
		current_offset += blob_size;
		message.blobs[key] = blob_info;
	}

	return append_blobs_to_string(JSON.stringify(message), blobs);
}

function split_text_and_data(array)
{
	let i = find(array, 0);
	return {body: new TextDecoder().decode(array.slice(0, i)), attachment_data: array.slice(i, array.length)};
}

function send_request(url, method = "GET", fields, blobs = {})
{
	return fetch(url, {
		method: method,
		redirect: "error",
		body: fields? make_request_body(fields, blobs) : null
	}).then(function(res) {
		return {succeeded: res.ok, pending_message: res.arrayBuffer()};
	}).then(async function(data){
		let message = split_text_and_data(new Uint8Array(await data.pending_message));
		console.log(message.attachment_data);
		return {succeeded: data.succeeded, message: JSON.parse(message.body)};
	});
}

function get_cookies(cookie_string)
{
	let elems = cookie_string.split("; ");
	let ret = {};
	for(let item in elems)
	{
		let parts = elems[item].split("=");
		if(parts.length == 2)
		{
			ret[parts[0]] = parts[1];
		}
	}

	return ret;
}

function clear_cookie(doc, name, hostname)
{
	doc.cookie = name + "=; Max-Age=0; SameSite=Strict; path=/; domain=" + hostname;
}

function file_to_array_buffer(file)
{
	return new Promise(function(resolve, reject) {
		let reader = new FileReader();
		reader.onload = function(){
			resolve(reader.result);
		};
		reader.onerror = reject;
		reader.readAsArrayBuffer(file);
	});
}