"use strict";

function create_input(schema_node)
{

}



let dummy_response = JSON.parse("{\
	\"fields\": {\
		\"name\": {\
			\"type\": \"restore:string\",\
			\"layout\": \"short\",\
			\"type_parameters\": {\
				\"present_as\": \"body_text\"\
			}\
		},\
		\"home_world\": {\
			\"type\": \"planet\"\
		}\
	},\
	\"types\": {\
		\"planet\": {\
			\"fields\": {\
				\"name\": {\
					\"type\": \"restore:string\",\
					\"layout\": \"short\",\
					\"type_parameters\": {\
						\"present_as\": \"body_text\"\
					}\
				},\
				\"mass\": {\
					\"type\": \"restore:float\",\
					\"layout\": \"short\",\
					\"type_parameters\": {\
						\"input_type\": \"text\"\
					}\
				},\
				\"radius\": {\
					\"type\": \"restore:float\",\
					\"layout\": \"short\",\
					\"type_parameters\": {\
						\"input_type\": \"text\"\
					}\
				}\
			}\
		}\
	}\
}");

console.log(dummy_response)