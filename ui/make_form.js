"use strict";

let strcmp = new Intl.Collator(undefined, {numeric:true, sensitivity: "base"}).compare;

function field_type_is_builtin(typeid)
{
	return typeid.split(":")[0] === "restore";
}

function type_is_long(type)
{
	let name_split = type.id.split(":");

	if(field_type_is_builtin(type.id))
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

function create_text_area(element_factory)
{
	let element = element_factory.createElement("textarea");
	element.setAttribute("rows", "24");
	element.setAttribute("cols", "80");
	return element;
}

function create_builtin_input_field(element_factory, field)
{
	let typeid_split = field.second.type.id.split(":");
	if(typeid_split[0] !== "restore")
	{ throw new Error("Type is not builtin"); }

	if(typeid_split.length !== 2)
	{ throw new Error("Type is not builtin"); }

	let typeid = typeid_split[1];

	if(typeid === "string")
	{
		let element = type_is_long(field.second.type)?
			create_text_area(element_factory):
			element_factory.createElement("input");

		element.setAttribute("name", field.first);
		element.setAttribute("value-type", typeid);

		return element;
	}
	else
	{
		let element = element_factory.createElement("input");
		element.setAttribute("name", field.first);
		element.setAttribute("value-type", typeid);
		return element;
	}
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

	let last_short = fields.findIndex(function(element){
		return type_is_long(element.second.type) === 1;
	});
	if(last_short === -1)
	{ last_short = fields.length; }

	let table = element_factory.createElement("table");
	for(let k = 0; k != last_short; ++k)
	{
		let row = element_factory.createElement("tr");

		let header = element_factory.createElement("th");
		let header_content = element_factory.createTextNode(fields[k].first);
		header.appendChild(header_content);
		row.appendChild(header);

		let input_field = element_factory.createElement("td");
		if(field_type_is_builtin(fields[k].second.type.id))
		{ input_field.appendChild(create_builtin_input_field(element_factory, fields[k])); }

		// TODO: Implement support for custom types
		// field_record_description = get_record_description(fields[k].second.type)

		row.appendChild(input_field);

		table.appendChild(row);
	}
	output_element.appendChild(table);

	for(let k = last_short; k != fields.length; ++k)
	{
		let container = element_factory.createElement("section");

		let container_title = element_factory.createElement("h1");
		let container_title_content = element_factory.createTextNode(fields[k].first);
		container_title.appendChild(container_title_content);
		container.appendChild(container_title);

		if(field_type_is_builtin(fields[k].second.type.id))
		{ container.appendChild(create_builtin_input_field(element_factory, fields[k])); }

		// TODO: Implement support for custom types
		// field_record_description = get_record_description(fields[k].second.type)

		output_element.appendChild(container);
	}
}