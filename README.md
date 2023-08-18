# Date and Time

There are various standards for expressing 
[dates and times](https://www.ipses.com/eng/in-depth-analysis/standard-of-time-definition/).
For financial events we use _Coordinated Universal Time_, also know as UTC.
Why is it not called CUT? As with most date and time related issues there
is a reason, but the details are rarely important.

A _time point_ is an absolute point in time. 
A _duration_ is a difference between two time points.
Given two time points $t_0$ and $t_1$ the duration is $d = t_1 - t_0$.
A time point plus a duration is a time point and must satisfy $t_0 + d = t_1$.

A _date_ is a time point with a resolution of one day and a difference of dates is the number of days
between the two dates. A _field-based_ representation of date contains the _year_, _month_, and _day_.
A _serial-based_ representation of date is the number of days since some _epoch_ date.
On Unix systems this is often January 1, 1970.

A _time_ is a duration since midnight on a given date. A field-based
representation might include _hours_, _minutes_, _seconds_ and possibly
milli or microseconds depending on the resolution. A serial-base representation
is an integral muliple of the finest resolution for the time.

We use the C++ [`<chrono>`](https://en.cppreference.com/w/cpp/chrono) library that provides
additional relevant utilities such as day-of-week, end-of-month/week, and conversion
from UTC to the local time of the computer running the code.

## Time Point

The library provides the class 
[`std::chrono::time_point<Clock, Duration>`](https://en.cppreference.com/w/cpp/chrono/time_point)
parameterized by a clock and a duration. There are many clocks provided by
the library but we use 
[`std::chrono::system_clock`](https://en.cppreference.com/w/cpp/chrono/system_clock)
which represents the system-wide real time wall clock.

## Duration

Every clock has a resolution specified by a
[`std::chrono::duration<Rep, Period>`](https://en.cppreference.com/w/cpp/chrono/duration)
where `Rep` is an arithmetic type and `Period` is a 
[`std::ratio`](https://en.cppreference.com/w/cpp/numeric/ratio/ratio)
representing the time in seconds from one tick to the next.

[`std::chrono::system_clock::time_point`] which is a time point with a resolution of std::chrono::system_clock::period (typically on the order of a microsecond or nanosecond).

```
	auto d0 = year{2023}/1/1;
	auto d1 = year{2024}/1/3;

```