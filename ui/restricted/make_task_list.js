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
		show_error_message(element_factory,
			"The server returned HTTP status " +
			response.message.http_status.toString() + " with message " + response.message.error_message,
			output_container);
		return;
	}

	if(Object.keys(response.message).length == 0)
	{
		show_error_message(element_factory, "Task list is empty", output_container);
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
			let button = element_factory.createElement("input");
			button.setAttribute("type", "button");
			button.addEventListener("mouseup", function(){
				row_event_handler.get_parameters(task, task_uri_name);
			});
			button.setAttribute("value", "Get parameters");
			cell.appendChild(button);
			row.appendChild(cell);
		}


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
			button.addEventListener("mouseup", function(){
				row_event_handler.resume(task, task_uri_name);  // TODO: Depends on task status
			});
			button.setAttribute("value", "Resume");  // TODO: Depends on task status
			cell.appendChild(button);
			row.appendChild(cell);
		}

		{
			let cell = element_factory.createElement("td");
			let button = element_factory.createElement("input");
			button.setAttribute("type", "button");
			button.addEventListener("mouseup", function(){
				row_event_handler.take_snapshot(task, task_uri_name);
			});
			button.setAttribute("value", "Take snapshot");
			cell.appendChild(button);
			row.appendChild(cell);
		}

		{
			let cell = element_factory.createElement("td");
			let button = element_factory.createElement("input");
			button.setAttribute("type", "button");
			button.addEventListener("mouseup", function(){
				row_event_handler.reset(task, task_uri_name);
			});
			button.setAttribute("value", "Reset");
			cell.appendChild(button);
			row.appendChild(cell);
		}

		{
			let cell = element_factory.createElement("td");
			let button = element_factory.createElement("input");
			button.setAttribute("type", "button");
			button.addEventListener("mouseup", function(){
				row_event_handler.clone(task, task_uri_name);
			});
			button.setAttribute("value", "Clone");
			cell.appendChild(button);
			row.appendChild(cell);
		}

		{
			let cell = element_factory.createElement("td");
			let button = element_factory.createElement("input");
			button.setAttribute("type", "button");
			button.addEventListener("mouseup", function(){
				row_event_handler.delete(task, task_uri_name);
			});
			button.setAttribute("value", "Delete");
			cell.appendChild(button);
			row.appendChild(cell);
		}

		output_container.appendChild(row);
	}
}