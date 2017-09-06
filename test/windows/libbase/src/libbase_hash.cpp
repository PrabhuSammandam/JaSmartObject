#include "libbase_hash.h"
#include "cute.h"
#include "Hash.h"

using namespace ja_iot::base;

void hash_test_1() {
//	ASSERTM("start writing tests", false);
	uint32_t buffer = 0x01203412;
	uint32_t hash_value = Hash::get_hash(&buffer, sizeof(buffer));
}

cute::suite make_suite_libbase_hash() {
	cute::suite s { };
	s.push_back(CUTE(hash_test_1));

	return s;
}
