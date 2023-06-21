# RESTful Task cOntrollER

## API endpoint summary

| Endpoint                       | Method  | Result                                                 |
|--------------------------------|---------|--------------------------------------------------------|
| /task_parameters               | GET     | Returns a list of all task parameters                  |
| /parameter_types               | GET     | Returns a list of all custom types used for parameters |
| /tasks                         | GET     | Returns a list of all tasks                            |
| /task/«task name»              | POST    | Creates «task name»                                    |
| /task/«task name»              | DELETE  | Deletes «task name»                                    |
| /task/«task name»/status       | GET     | Gets the process status of «task name»                 |
| /task/«task name»/status       | PUT     | Use to suspend/resume «task name»                      |
| /task/«task name»/progress     | GET     | Tracks the progress of «task name»                     |
| /task/«task name»/result       | GET     | Dumps the latest result of «task name»                 |