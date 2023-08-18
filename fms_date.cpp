// fms_date.cpp
#include "fms_date.h"

using namespace std::literals;
using namespace fms::date;

int test_date = fms::date::test();
periodic<> p;
periodic_iterable pi(p);
int test_periodic = pi.test();
int main()
{
	return 0;
}