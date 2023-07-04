//@	{"dependencies_extra": [{"ref":"./dummy_params.o", "rel":"implementation"}]}

#ifndef RESTORE_SERVER_DUMMYPARAMS_HPP
#define RESTORE_SERVER_DUMMYPARAMS_HPP

#include "./jopp_wrapper.hpp"
#include "./task.hpp"

#include <jopp/types.hpp>
#include <thread>

namespace restore
{
	jopp::object get_parameter_types();
	jopp::object get_task_parameters();

	struct dummy_task
	{
		double get_progress() const
		{ return static_cast<double>(iter_count)/static_cast<double>(iter_count_max); }

		void set_parameters(json::object_ref){}

		void dump_state(int) const {}

		void set_state(int){}

		void reset(){ iter_count = 0; }

		task_step_result step()
		{
			if(iter_count != iter_count_max)
			{
				++iter_count;
				std::this_thread::sleep_for(std::chrono::seconds{10});
				return task_step_result::keep_going;
			}

			return task_step_result::task_is_completed;
		}

		size_t iter_count{0};
		size_t iter_count_max{1024};
	};
}

#endif