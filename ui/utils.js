"use strict";

let strcmp = new Intl.Collator(undefined, {numeric:true, sensitivity: "base"}).compare;

function find_if(array, predicate)
{
	let ret = array.findIndex(predicate);
	return ret === - 1? array.length : ret;
}

function fetch_data(url)
{
	return fetch("/tasks", {
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