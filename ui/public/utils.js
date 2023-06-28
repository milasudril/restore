"use strict";

let strcmp = new Intl.Collator(undefined, {numeric:true, sensitivity: "base"}).compare;

function find_if(array, predicate)
{
	let ret = array.findIndex(predicate);
	return ret === - 1? array.length : ret;
}

function fetch_data(url)
{
	return fetch(url, {
		headers: {
			"Accept": "application/json"
		}
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
