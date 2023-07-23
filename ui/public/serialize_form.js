"use strict";

async function serialize_value(input_field, type_name, blobs, path)
{
	if(type_name === "string")
	{ return input_field.value; }

	if(type_name === "number")
	{ return parseFloat(input_field.value); }

	if(type_name === "boolean")
	{ return input_field.checked; }

	if(type_name === "blob")
	{
		if(input_field.files[0] !== undefined)
		{
			let blob_name = input_field.files[0].name + "_" + blobs.length.toString();
			blobs.values[blob_name] = {
				data: await file_to_array_buffer(input_field.files[0]),
				json_path: path
			};
			blobs.length += 1;
			return blob_name;
		}
		return null;
	}

	return input_field.value;
}

async function serialize_form(subform, output_object, blobs, path = [])
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
						let new_path = Array.from(path);
						new_path.push(field_name);
						await serialize_form(field, output_object[field_name], blobs, new_path);
					}
					else
					{ output_object[field_name] = await serialize_value(field.children[0], field_type_name, blobs, path); }
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
				let new_path = Array.from(path);
				new_path.push(field_name);
				await serialize_form(field, output_object[field_name], blobs, new_path);
			}
			else
			{ output_object[field_name] = await serialize_value(field.children[1], field_type_name, blobs, path); }
		}
	}
}