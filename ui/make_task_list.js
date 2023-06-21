"use strict";

function fill_tasklist(response, element_factory, output_container)
{
	if(!response.succeeded)
	{
		let caption = element_factory.createElement("caption");
		caption.setAttribute("class", "error_message");
		let caption_content = element_factory.createTextNode("There are tasks to display: " + response.message.error_message);
		caption.appendChild(caption_content);
		output_container.appendChild(caption);
		return;
	}

	if(Object.keys(response).length == 0)
	{
		let caption = element_factory.createElement("caption");
		caption.setAttribute("class", "error_message");
		let caption_content = element_factory.createTextNode("There are no tasks to display");
		caption.appendChild(caption_content);
		output_container.appendChild(caption);
		return;
	}

	for(let task in response.message)
	{
		let row = element_factory.createElement("tr");

		let taskname_cell = element_factory.createElement("td");
		let taskname_element = element_factor.createTextNode(task);
		taskname_cell.appendChild(taskname_element);
		row.appendChild(taskname_cell);

		let taskstatus_cell = element_factory.createElement("td");
		let taskstatus_element = element_factor.createTextNode(response.message[task].status);
		taskstatus_cell.appendChild(taskstatus_element);
		row.appendChild(taskstatus_cell);

		output_container.appendChild(row);
	}
}