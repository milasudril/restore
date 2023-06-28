"use strict";

function serialize_value(input_field, type_name)
{
	if(type_name === "string")
	{ return input_field.value; }

	if(type_name === "number")
	{ return parseFloat(input_field.value); }

	if(type_name === "boolean")
	{ return input_field.checked; }

	return input_field.value;
}

function serialize_form(subform, output_object)
{
	for(let item in subform.children)
	{
		let child = subform.children[item];
		let tag_name = child.tagName;
		if(tag_name === "TABLE" || tag_name === "table")
		{
			for(let row in child.rows)
			{
				let cells = child.rows[row].cells;
				if(cells)
				{
					let field = cells[1];
					let field_name = field.getAttribute("field-name");
					let field_type_name = field.getAttribute("field-type-name");
					let field_type_category = field.getAttribute("field-type-category");

					if(field_type_category === "composite")
					{
						output_object[field_name] = {};
						serialize_form(field, output_object[field_name]);
					}
					else
					{ output_object[field_name] = serialize_value(field.children[0], field_type_name); }
				}
			}
		}
		else
		if(tag_name === "SECTION" || tag_name === "section")
		{
			let field = child;

			let field_name = field.getAttribute("field-name");
			let field_type_name = field.getAttribute("field-type-name");
			let field_type_category = field.getAttribute("field-type-category");

			if(field_type_category === "composite")
			{
				output_object[field_name] = {};
				serialize_form(field, output_object[field_name]);
			}
			else
			{ output_object[field_name] = serialize_value(field.children[1], field_type_name); }
		}
	}
}