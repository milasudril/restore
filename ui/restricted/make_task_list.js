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

function running_status_to_label(status)
{
	if(status === "suspended")
	{ return "Resume"; }
	if(status === "running")
	{ return "Suspend"; }

	return "Reset";
}

function fill_tasklist(response, element_factory, output_container, row_event_handler)
{
	let ret = [];

	if(!response.succeeded)
	{
		show_error_message(element_factory,
			"The server returned HTTP status " +
			response.message.http_status.toString() + " with message " + response.message.error_message,
			output_container);
		return ret;
	}

	if(Object.keys(response.message).length == 0)
	{
		show_error_message(element_factory, "Task list is empty", output_container);
		return ret;
	}

	for(let task_name in response.message)
	{
		let row = element_factory.createElement("tr");

		{
			let cell = element_factory.createElement("td");
			let cell_text = element_factory.createTextNode(task_name);
			cell.appendChild(cell_text);
			row.appendChild(cell);
		}

		let task = response.message[task_name];
		let task_uri_name = response.message[task_name].uri_name;

		{
			let cell = element_factory.createElement("td");
			let button = element_factory.createElement("input");
			button.setAttribute("type", "button");
			button.addEventListener("mouseup", function(){
				row_event_handler.get_parameters(task_name, task_uri_name);
			});
			button.setAttribute("value", "Get parameters");
			cell.appendChild(button);
			row.appendChild(cell);
		}

		let progress_bar = {};
		{
			let cell = element_factory.createElement("td");
			let progress = element_factory.createElement("progress");
			progress.setAttribute("restore-task-ref", task_uri_name);
			progress.setAttribute("restore-task-name", task_name);
			progress.setAttribute("value", task.progress.toString());
			cell.appendChild(progress);
			row.appendChild(cell);
			progress_bar = progress;
		}

		let status_button = {};
		{
			let cell = element_factory.createElement("td");
			let button = element_factory.createElement("input");
			button.setAttribute("type", "button");
			button.addEventListener("mouseup", async function(event){
				let button = event.target;
				let state = button.getAttribute("restore-task-rs");

				let response = {};
				if(state === "suspended")
				{ response = await row_event_handler.resume(task_name, task_uri_name); }

				if(state === "running")
				{ response = await row_event_handler.suspend(task_name, task_uri_name); }

				if(state === "completed")
				{ response = await row_event_handler.reset(task_name, task_uri_name); }

				if(!response.succeeded)
				{
					alert("The server returned HTTP status " +
						response.message.http_status.toString() +
						" with message " + response.message.error_message);
					return;
				}

				let running_status = response.message.result;
				button.setAttribute("value", running_status_to_label(running_status));
				button.setAttribute("restore-task-rs", running_status);
			});
			button.setAttribute("value", running_status_to_label(task.running_status));
			button.setAttribute("restore-task-rs", task.running_status);
			cell.appendChild(button);
			row.appendChild(cell);
			status_button = button;
		}

		{
			let cell = element_factory.createElement("td");
			let button = element_factory.createElement("input");
			button.setAttribute("type", "button");
			button.addEventListener("mouseup", function(){
				row_event_handler.take_snapshot(task_name, task_uri_name);
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
		ret.push({uri_name: task_uri_name, progress_bar: progress_bar, status_button: status_button});
	}
	return ret;
}