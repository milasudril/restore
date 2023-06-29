# RESTful Task cOntrollER

## API endpoint summary

| Endpoint                            | Method | Result                                                 |
|-------------------------------------|--------|--------------------------------------------------------|
| /task_parameters                    | GET    | Returns a list of all task parameters                  |
| /parameter_types                    | GET    | Returns a list of all custom types used for parameters |
| /tasks                              | GET    | Returns a list of all tasks                            |
| /tasks                              | POST   | Creates «task name»                                    |
| /tasks/«encoded task name»          | DELETE | Deletes «task name»                                    |
| /tasks/«encoded task name»/status   | GET    | Gets the process status of «task name»                 |
| /tasks/«encoded task name»/status   | PUT    | Use to suspend/resume «task name»                      |
| /tasks/«encoded task name»/progress | GET    | Tracks the progress of «task name»                     |
| /tasks/«encoded task name»/result   | GET    | Dumps the latest result of «task name»                 |


## Data file organization

| File                                     | Description                                                       |
|------------------------------------------|-------------------------------------------------------------------|
| restore_metadata.json                    | Identifies the file, what task processor the file is intended for |
| shared/tasks/«task name»/parameters.json | Task parameters                                                   |
| shared/tasks/«task name»/state.wad64     | The state of current task                                         |