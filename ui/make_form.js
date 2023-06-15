"use strict";

function generate_form(element_factory, output_element, record_description, types)
{
	let fields = new Array();
	for(let item in record_description)
	{
		fields.push({"first": item, "second": record_description[item]})
	}

	for(let item in fields)
	{
		let container = element_factory.createElement("p");
		let container_content = element_factory.createTextNode(fields[item].first);
		container.appendChild(container_content);
		output_element.appendChild(container);
	}
}