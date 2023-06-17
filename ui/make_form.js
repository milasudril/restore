"use strict";

let strcmp = new Intl.Collator(undefined, {numeric:true, sensitivity: "base"}).compare;

function field_type_is_builtin(type)
{
	return type.category === "atom";
}

function show_as_paragraph(field)
{
	if(field.display === "block")
	{ return 1;}
	else
	if(field.display === "inline")
	{ return 0; }

	throw new Error("Invalid display mode `" + field.display + "`");
}

function create_text_area(element_factory)
{
	let element = element_factory.createElement("textarea");
	element.setAttribute("rows", "8");
	return element;
}

function create_input_field_atom(element_factory, type)
{
	if(type.name === "string")
	{
		let element = type.input_size === "long"?
			create_text_area(element_factory):
			element_factory.createElement("input");
		return element;
	}
	else
	if(type.name === "boolean")
	{
		let element = element_factory.createElement("input");
		element.setAttribute("type", "checkbox");
		return element;
	}
	else
	{
		let element = element_factory.createElement("input");
		return element;
	}
}

function create_input_field_enum(element_factory, type)
{
	let element = element_factory.createElement("select")

	for(let item in type.allowed_values)
	{
		let option = element_factory.createElement("option");
		option.setAttribute("value", item);
		let entry = element_factory.createTextNode(type.allowed_values[item].display_name?
			type.allowed_values[item].display_name : item);
		option.appendChild(entry);
		element.appendChild(option);
	}

	return element;
}


function create_input_field(element_factory, parent_element, field, types)
{
	let type = field.second.type;
	parent_element.setAttribute("field-name", field.first);
	parent_element.setAttribute("field-type-name", type.name);
	parent_element.setAttribute("field-type-category", type.category);

	if(type.category === "atom")
	{
		parent_element.appendChild(create_input_field_atom(element_factory, type));
		return;
	}

	if(type.category === "composite")
	{
		generate_form(element_factory, parent_element, types.composite[type.name].fields, types);
		return;
	}

	if(type.category === "enum")
	{
		parent_element.appendChild(create_input_field_enum(element_factory, types.enum[type.name]));
		return;
	}

	throw Error("Unsupported type category `" + type.category + "`");
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
		header.setAttribute("class", "row-header");
		let header_content = element_factory.createTextNode(fields[k].first);
		header.appendChild(header_content);
		row.appendChild(header);

		let input_field_container = element_factory.createElement("td");

		create_input_field(element_factory, input_field_container, fields[k], types);

		row.appendChild(input_field_container);
		table.appendChild(row);
	}
	output_element.appendChild(table);

	for(let k = last_inline; k != fields.length; ++k)
	{
		let input_field_container = element_factory.createElement("section");

		let container_title = element_factory.createElement("h1");
		let container_title_content = element_factory.createTextNode(fields[k].first);
		container_title.appendChild(container_title_content);
		input_field_container.appendChild(container_title);

		create_input_field(element_factory, input_field_container, fields[k], types);

		output_element.appendChild(input_field_container);
	}
}