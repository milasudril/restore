"use strict";

function generate_form(element_factory, output_element, record_description, types)
{
	for(let item in record_description)
	{
		let container = element_factory.createElement("p");
		let container_content = element_factory.createTextNode(item);
		container.appendChild(container_content);
		output_element.appendChild(container);
	}
}