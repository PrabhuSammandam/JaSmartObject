#include "libbase_ptr_array.h"
#include "cute.h"

#include "PtrArray.h"

using namespace ja_iot::base;

void ptr_array_test_1() {
	uint32_t*u32_array[10] { };

	PtrArray<uint32_t*> ptr_array(&u32_array[0], 10);

	ptr_array.Add((uint32_t*)10);
	ptr_array.Add((uint32_t*)20);

	ASSERTM("Ptr Array Count ", ptr_array.Count() == 2);
	ASSERTM("Ptr Array Index for 1st item", ptr_array.GetIndex((uint32_t*)10) == 0);
	ASSERTM("Ptr Array Index for 2st item", ptr_array.GetIndex((uint32_t*)20) == 1);
}

cute::suite make_suite_libbase_ptr_array() {
	cute::suite s { };
	s.push_back(CUTE(ptr_array_test_1));
	return s;
}
