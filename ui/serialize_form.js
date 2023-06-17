"use strict";

function serialize_form(subform, output_object)
{
	for(let item in subform.children)
	{
		let child = subform.children[item];
		let tag_name = child.tagName;
		if(tag_name === "TABLE" || tag_name === "table")
		{
			let rows = child.children[0].children;  // Dive into generated tbody
			for(let row in rows)
			{
				let cells = rows[row].children;
				if(cells)
				{
					let item = cells[1];
					let field_name = item.attributes["field-name"].value;
					let input_field = item.children[0];
					let field_type = input_field.attributes["value-type"].value;
					let field_value = item.children[0].value;
					output_object[field_name] = field_value;
				}
			}
		}
		else
		if(tag_name === "SECTION" || tag_name === "section")
		{
//			console.log("section");
		}
	}
}