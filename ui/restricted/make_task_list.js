"use strict";

function show_error_message(element_factory, text, output_container)
{
	let row = element_factory.createElement("tr");
	let cell = element_factory.createElement("td");
	cell.setAttribute("class", "error_message");
	let message = element_factory.createTextNode("There are tasks to display: " + text);
	cell.appendChild(message);
	row.appendChild(message);
	output_container.appendChild(row);
}

function fill_tasklist(response, element_factory, output_container)
{
	if(!response.succeeded)
	{
		show_error_message(element_factory, response.message.error_message, output_container);
		return;
	}

	if(Object.keys(response).length == 0)
	{
		show_error_message(element_factory, "No error", output_container);
		return;
	}

	for(let task in response.message)
	{
		let row = element_factory.createElement("tr");

		let taskname_cell = element_factory.createElement("td");
		let taskname_element = element_factory.createTextNode(task);
		taskname_cell.appendChild(taskname_element);
		row.appendChild(taskname_cell);

		let taskstatus_cell = element_factory.createElement("td");
		let taskstatus_element = element_factor.createTextNode(response.message[task].status);
		taskstatus_cell.appendChild(taskstatus_element);
		row.appendChild(taskstatus_cell);

		output_container.appendChild(row);
	}
}