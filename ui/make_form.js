"use strict";

let strcmp = new Intl.Collator(undefined, {numeric:true, sensitivity: "base"}).compare;

function field_type_is_builtin(typeid)
{
	return typeid.split(":")[0] === "restore";
}

function show_as_paragraph(field)
{
	if(field.display === "block")
	{ return 1;}
	else
	if(field.display === "inline")
	{ return 0; }

	throw new Error("Invalid display type");
}

function create_text_area(element_factory)
{
	let element = element_factory.createElement("textarea");
	element.setAttribute("rows", "8");
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
		let element = field.second.type.layout === "long"?
			create_text_area(element_factory):
			element_factory.createElement("input");
		element.setAttribute("value-type", typeid);
		return element;
	}
	else
	{
		let element = element_factory.createElement("input");
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
		let a_is_paragraph = show_as_paragraph(a.second);
		let b_is_paragraph = show_as_paragraph(b.second);

		if(a_is_paragraph < b_is_paragraph)
		{ return -1; }

		if(a_is_paragraph > b_is_paragraph)
		{ return 1; }

		if(a_is_paragraph === b_is_paragraph)
		{ return strcmp(a.first, b.first); }
	});

	let last_inline = fields.findIndex(function(element){
		return show_as_paragraph(element.second) === 1;
	});
	if(last_inline === -1)
	{ last_inline = fields.length; }

	let table = element_factory.createElement("table");
	for(let k = 0; k != last_inline; ++k)
	{
		let row = element_factory.createElement("tr");

		let header = element_factory.createElement("th");
		let header_content = element_factory.createTextNode(fields[k].first);
		header.appendChild(header_content);
		row.appendChild(header);

		let input_field = element_factory.createElement("td");
		if(field_type_is_builtin(fields[k].second.type.id))
		{ input_field.appendChild(create_builtin_input_field(element_factory, fields[k])); }
		else
		{ generate_form(element_factory, input_field, types[fields[k].second.type.id].fields, types); }

		input_field.setAttribute("field-name", fields[k].first);
		row.appendChild(input_field);
		table.appendChild(row);
	}
	output_element.appendChild(table);

	for(let k = last_inline; k != fields.length; ++k)
	{
		let container = element_factory.createElement("section");

		let container_title = element_factory.createElement("h1");
		let container_title_content = element_factory.createTextNode(fields[k].first);
		container_title.appendChild(container_title_content);
		container.appendChild(container_title);

		if(field_type_is_builtin(fields[k].second.type.id))
		{ container.appendChild(create_builtin_input_field(element_factory, fields[k])); }
		else
		{ generate_form(element_factory, container, types[fields[k].second.type.id].fields, types); }

		container.setAttribute("field-name", fields[k].first);
		output_element.appendChild(container);
	}
}