// fms_date.cpp
#ifdef _DEBUG
#include <cassert>
#include "fms_date.h"

using namespace fms::date;

int test_basic_date = fms::date::basic_date_test();
int test_date_dcf = fms::date::dcf::test();
int test_date = fms::date::test();
int test_periodic = periodic_test();
#endif // _DEBUG

int main()
{
	return 0;
}