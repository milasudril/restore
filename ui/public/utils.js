"use strict";

let strcmp = new Intl.Collator(undefined, {numeric:true, sensitivity: "base"}).compare;

function find_if(array, predicate)
{
	let ret = array.findIndex(predicate);
	return ret === - 1? array.length : ret;
}

function append_data_to_string(str, data)
{
	let encoded_string = new TextEncoder().encode(str);
	if(data.length === 0)
	{ return encoded_string; }

	let ret = new Uint8Array(encoded_string.length + data.length + 1);
	ret.set(encoded_string, 0);
	ret.set(data, encoded_string.length + 1)

	return ret;
}

function uint8_array_to_array_buffer(array)
{
	return array.buffer.slice(array.byteOffset, array.byteLength + array.byteOffset);
}

function send_request(url, method = "GET", body, attachment=new ArrayBuffer())
{
	return fetch(url, {
		method: method,
		redirect: "error",
		headers: {
			"Accept": "application/json"
		},
		body: body? append_data_to_string(JSON.stringify(body), new Uint8Array(attachment)) : null
	}).then(function(res) {
		return {succeeded: res.ok, pending_message: res.json()};
	}).then(async function(data){
		let message = await data.pending_message;
		return {succeeded: data.succeeded, message: message};
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