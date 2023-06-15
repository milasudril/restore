"use strict";

let strcmp = new Intl.Collator(undefined, {numeric:true, sensitivity: "base"}).compare;

function type_is_long(type)
{
	let name_split = type.id.split(":");
	console.log(name_split[0])

	if(name_split[0] === "restore")
	{
		if(type.parameters.layout)
		{
			if(type.parameters.layout === "long")
			{ return 1;}
		}
		return 0;
	}

	// For now custom types are always considered as long
	return 1;
}

function generate_form(element_factory, output_element, record_description, types)
{
	let fields = new Array();
	for(let item in record_description)
	{ fields.push({"first": item, "second": record_description[item]}); }

	fields.sort(function(a, b){
		let a_is_short = type_is_long(a.second.type);
		let b_is_short = type_is_long(b.second.type);

		if(a_is_short < b_is_short)
		{ return -1; }

		if(a_is_short > b_is_short)
		{ return 1; }

		if(a_is_short === b_is_short)
		{ return strcmp(a.first, b.first); }
	});

	for(let item in fields)
	{
		let container = element_factory.createElement("p");
		let container_content = element_factory.createTextNode(type_is_long(fields[item].second.type) + " " + fields[item].first);
		container.appendChild(container_content);
		output_element.appendChild(container);
	}
}