# RESTful Task cOntrollER

## API endpoints

| Endpoint                                  | Method | Result                                                 |
|-------------------------------------------|--------|--------------------------------------------------------|
| /task_parameters                          | GET    | Returns a list of all task parameters                  |
| /parameter_types                          | GET    | Returns a list of all custom types used for parameters |
| /tasks                                    | GET    | Returns a list of all tasks                            |
| /tasks                                    | POST   | Creates «task name»                                    |
| /tasks/«encoded task name»                | DELETE | Deletes «task name»                                    |
| /tasks/«encoded task name»                | POST   | Creates a deep copy of «task name»                     |
| /tasks/«encoded task name»/parameters     | GET    | Dumps the parameters with value of «task name»         |
| /tasks/«encoded task name»/system_state   | GET    | Dumps the latest result of «task name»                 |
| /tasks/«encoded task name»/system_state   | DELETE | Resets the system state of «task name»                 |
| /tasks/«encoded task name»/running_status | GET    | Check if «task name» running or suspended              |
| /tasks/«encoded task name»/running_status | PUT    | Use to suspend/resume «task name»                      |
| /tasks/«encoded task name»/progress       | GET    | Tracks the progress of «task name»                     |


### /tasks/«encoded task name»/system_state (GET)

Maps to `void` «Task»`::take_snapshot(Wad64::OutputFile& ouptut_file) const`


### /tasks/«encoded task name»/system_state (DELETE)

Maps to «Task»`::`«Task»`(jopp::object const& params, Wad64::InputFile const& initial_state)`, with `initial_state`
state being an empty file.


### /tasks/«encoded task name»/running_status (PUT)

| Argument                | API entry point                                                                |
|-------------------------|--------------------------------------------------------------------------------|
| `{"mode": "running"}`   | «Task»`::`«Task»`(jopp::object const&, Wad64::InputFile const& initial_state)` |
| `{"mode": "suspended"}` | Internal call that causes the `restore` processing loop to return              |


### /tasks/«encoded task name»/progress

Maps to `restore::progress` «Task»`::get_progress() const`


## Data file organization

| File                                     | Description                                                       |
|------------------------------------------|-------------------------------------------------------------------|
| restore_metadata.json                    | Identifies the file, what task processor the file is intended for |
| shared/tasks/«task name»/parameters.json | Task parameters                                                   |
| shared/tasks/«task name»/state.wad64     | The state of current task                                         |