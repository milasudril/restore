"use strict";

/*
let strcmp = new Intl.Collator(undefined, {numeric:true, sensitivity: "base"}).compare;

function is_short(type_name, type_params)
{
	return 1;
}

function is_short(type)
{
	type_parts = type.split(":");
	if(type_parts[0] === "restore")
	{ return is_short(type_parts[1], type.type_parameters); }

	// A user-defined type is considered "long" for now
	return 0;
}
*/

function generate_form(element_factory, output_element, record_description, types)
{
	let fields = new Array();
	for(let item in record_description)
	{ fields.push({"first": item, "second": record_description[item]}); }
/*
	fields.sort(function(a, b){
		let a_is_short = is_short(a.second.type, a.second.type_parameters);
		let b_is_short = is_short(b.second.type, b.second.type_parameters);

		if(a_is_short < b_is_short)
		{ return -1; }

		if(a_is_short > b_is_short)
		{ return 1; }

		if(a_is_short === b_is_short)
		{ return strcmp(b.first, a.first); }
	});
*/
	for(let item in fields)
	{
		let container = element_factory.createElement("p");
		let container_content = element_factory.createTextNode(fields[item].first);
		container.appendChild(container_content);
		output_element.appendChild(container);
	}
}