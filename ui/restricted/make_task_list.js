"use strict";

function show_error_message(element_factory, text, output_container)
{
	let row = element_factory.createElement("tr");
	let cell = element_factory.createElement("td");
	cell.setAttribute("class", "error_message");
	let message = element_factory.createTextNode("There are no tasks to display: " + text);
	cell.appendChild(message);
	row.appendChild(message);
	output_container.appendChild(row);
}

function fill_tasklist(response, element_factory, output_container, row_event_handler)
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

		{
			let cell = element_factory.createElement("td");
			let cell_text = element_factory.createTextNode(task);
			cell.appendChild(cell_text);
			row.appendChild(cell);
		}

		let task_uri_name = response.message[task].uri_name

		{
			let cell = element_factory.createElement("td");
			let progress = element_factory.createElement("progress");
			progress.setAttribute("restore-task-ref", task_uri_name);
			progress.setAttribute("restore-task-name", task);
			progress.setAttribute("value", "0.5");
			cell.appendChild(progress);
			row.appendChild(cell);
		}

		{
			let cell = element_factory.createElement("td");
			let button = element_factory.createElement("input");
			button.setAttribute("type", "button");
			button.setAttribute("restore-task-ref", task_uri_name);
			button.setAttribute("restore-task-name", task);
			button.addEventListener("mouseup", row_event_handler);
			button.setAttribute("restore-task-action", "get_result");
			button.setAttribute("value", "Get result");
			cell.appendChild(button);
			row.appendChild(cell);
		}

		{
			let cell = element_factory.createElement("td");
			let button = element_factory.createElement("input");
			button.setAttribute("type", "button");
			button.setAttribute("restore-task-ref", task_uri_name);
			button.setAttribute("restore-task-name", task);
			button.addEventListener("mouseup", row_event_handler);
			button.setAttribute("restore-task-action", "Resume");  // TODO: Depends on task status
			button.setAttribute("value", "Resume");  // TODO: Depends on task status
			cell.appendChild(button);
			row.appendChild(cell);
		}

		{
			let cell = element_factory.createElement("td");
			let button = element_factory.createElement("input");
			button.setAttribute("type", "button");
			button.setAttribute("restore-task-ref", task_uri_name);
			button.setAttribute("restore-task-name", task);
			button.addEventListener("mouseup", row_event_handler);
			button.setAttribute("restore-task-action", "delete");
			button.setAttribute("value", "Delete");
			cell.appendChild(button);
			row.appendChild(cell);
		}


/*
		TODO: Will need to GET status
*/

		output_container.appendChild(row);
	}
}