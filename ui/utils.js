"use strict";

let strcmp = new Intl.Collator(undefined, {numeric:true, sensitivity: "base"}).compare;

function find_if(array, predicate)
{
	let ret = array.findIndex(predicate);
	return ret === - 1? array.length : ret;
}
