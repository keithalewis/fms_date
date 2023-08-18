# Date and Time

There are various standards for expressing 
[dates and times](https://www.ipses.com/eng/in-depth-analysis/standard-of-time-definition/).
For financial events we use _Coordinated Universal Time_, also know as UTC.
Why is it not called CUT? As with most date and time related issues, there
is a reason, but the details are rarely important.

A _time point_ is an absolute point in time. 
A _duration_ is a difference between two time points.
Given two time points $t_0$ and $t_1$ the duration is $d = t_1 - t_0$.
A time point plus a duration is a time point and must satisfy $t_0 + d = t_1$.

A _date_ is a time point with a resolution of one day and a difference of dates is the number of days
between the two dates. A _field-based_ representation of date contains the _year_, _month_, and _day_.
A _serial-based_ representation of date is the number of days since some _epoch_ date.
On Unix systems this is often January 1, 1970.

