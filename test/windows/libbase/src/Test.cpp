#include "cute.h"
#include "ide_listener.h"
#include "xml_listener.h"
#include "cute_runner.h"
#include "libbase_hash.h"
#include "libbase_ptr_array.h"

void thisIsATest() {
//	ASSERTM("start writing tests", false);
}

bool runAllTests(int argc, char const *argv[]) {
	cute::suite s { };

	//TODO add your test here
	s.push_back(CUTE(thisIsATest));

	cute::xml_file_opener xmlfile(argc, argv);
	cute::xml_listener<cute::ide_listener<>> lis(xmlfile.out);

	auto runner = cute::makeRunner(lis, argc, argv);

	bool success = runner(s, "AllTests");
	cute::suite libbase_hash = make_suite_libbase_hash();
	success &= runner(libbase_hash, "libbase_hash");
	cute::suite libbase_ptr_array = make_suite_libbase_ptr_array();
	success &= runner(libbase_ptr_array, "libbase_ptr_array");
	return success;
}

int main(int argc, char const *argv[]) {
	return runAllTests(argc, argv) ? EXIT_SUCCESS : EXIT_FAILURE;
}
