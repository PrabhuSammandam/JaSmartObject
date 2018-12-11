#include <iostream>
#include "Uuid.h"
#include "OsalRandom.h"

using namespace std;
using namespace ja_iot::base;
using namespace ja_iot::osal;

void test_uuid()
{
	uint8_t buf[16];
	Uuid uuid1{};

	OsalRandom::get_random_bytes(buf, 16);

	uuid1.set_value(buf, 16);

	Uuid uuid2{ uuid1 };// copy constructor
	Uuid uuid3{ std::move(uuid2) };// move constructor

	Uuid uuid4 = uuid3;// assignment operator
	Uuid uuid5 = std::move(uuid4);// move operator

	if (uuid1 == uuid5)
	{
		printf("passed\n");
	}

	std::string uuid_string;

	uuid5 >> uuid_string;// to string

	Uuid uuid6{};

	auto str_len = uuid_string.size();

	cout << uuid_string << endl;

	uuid6 << uuid_string; // from string

	if (uuid1 == uuid6)
	{
		printf("passed\n");
	}

	if (!uuid6.is_nil())
	{
		printf("passed\n");
	}

	uuid6.clear();

	if (uuid6.is_nil())
	{
		printf("passed\n");
	}

	cout << uuid1.to_string() << endl;

	if (Uuid::is_valid_uuid_string(uuid_string))
	{
		printf("passed\n");
	}
}