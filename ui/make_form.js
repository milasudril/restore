"use strict";

function create_input_leaf(parent_element, field_name, field_type, parameters)
{
	if(field_type === "string")
	{
		let input_field = document.createElement("input");
		input_field.setAttribute("type", "text");
		input_field.setAttribute("name", field_name);
		parent_element.appendChild(input_field);
		return;
	}
	let input_field = document.createElement("restore-error");
	let error_message = document.createTextNode(`Unsupported field type «${field_type}»`)
	input_field.appendChild(error_message);
	parent_element.appendChild(input_field);
}

function create_input(parent_element, field_name, field, schema)
{
	if(field.layout === "short")
	{
		let the_row = document.createElement("tr");
		let row_header = document.createElement("th");
		row_header.setAttribute("class","row-header");
		let row_header_content = document.createTextNode(field_name);
		row_header.appendChild(row_header_content);
		the_row.appendChild(row_header);

		let row_content = document.createElement("td");
		let split_type = field.type.split(":");
		if(split_type[0] === "restore")
		{
			create_input_leaf(row_content, field_name, split_type[1], field.type_parameters);
		}
		else
		{
			let child_schema = schema.types[field.type];
			let row_content_content = document.createTextNode(child_schema);
			row_content.appendChild(row_content_content);
		}
		the_row.appendChild(row_content);

		parent_element.appendChild(the_row);
		console.log(the_row)
	}
	else
	{
	}
}