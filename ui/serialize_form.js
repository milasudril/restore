"use strict";

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
					{ output_object[field_name] = field.children[0].value; }
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
			{ output_object[field_name] = field.children[1].value; }
		}
	}
}