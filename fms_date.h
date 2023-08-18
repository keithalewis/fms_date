// fms_date.h - Date and time calculation
#pragma once
#include <chrono>

namespace fms::date {

	// Calendar date.
	using ymd = std::chrono::year_month_day;
	constexpr ymd make_ymd(int y, int m, int d)
	{
		return std::chrono::year_month_day(std::chrono::year(y), std::chrono::month(m), std::chrono::day(d));
	}


	// Convert ymd to sys_days time point using a clock.
	using days = std::chrono::sys_days;
	constexpr days make_days(int y, int m, int d)
	{
		return days(make_ymd(y, m, d));
	}

	// duration as double in years
	using years = std::chrono::duration<double, std::chrono::years::period>;

	// d0 - d1 in years
	constexpr years minus_years(const ymd& d0, const ymd& d1)
	{
		return years(days(d0) - days(d1));
	}
#ifdef _DEBUG
	constexpr int basic_date_test()
	{
		{
			constexpr auto d0 = make_days(2023, 4, 5);
			constexpr auto d1 = make_days(2024, 4, 5);
			constexpr auto dd = minus_years(d0, d1);
			static_assert(d1 + dd == d0);
			static_assert(d0 - dd == d1);
		}

		return 0;
	}
#endif // _DEBUG
	
	// Day count fraction in years from d0 to d1.
	constexpr years dcf_years(const ymd& d0, const ymd& d1)
	{
		return minus_years(d1, d0);
	}
	// dcf_30_360, ...


	constexpr bool is_weekday(const ymd& d)
	{
		auto w = std::chrono::year_month_weekday(d);
		auto wd = w.weekday();

		return (wd != std::chrono::Saturday) and (wd != std::chrono::Sunday);
	}
	
	namespace holiday {

		// Every year holiday on month and day
		constexpr bool month_day(const ymd& d, const std::chrono::month& month, const std::chrono::day& day)
		{
			return (d.month() == month) and (d.day() == day);
		}

		constexpr bool new_year_day(const ymd& d)
		{
			return month_day(d, std::chrono::January, std::chrono::day(1));
		}

		constexpr bool christmas_day(const ymd& d)
		{
			return month_day(d, std::chrono::December, std::chrono::day(25));
		}

	} // namespace holiday

	namespace calendars {

		constexpr bool weekday(const ymd& d)
		{
			return !is_weekday(d);
		}
		constexpr bool example(const ymd& d)
		{
			return weekday(d) or holiday::new_year_day(d);
		}

	} // namespace calendars

	using calendar = bool(*)(const ymd&);

	// Business day rolling conventions.
	enum class roll {
		none,
		previous, // previous business day
		following, // following business day
		modified_following, // following business day unless different month
	};

	constexpr ymd adjust(const days& date, roll convention, const calendar& cal = calendars::weekday)
	{
		switch (convention) {
		case roll::none:
			return date;
		case roll::previous:
			if (cal(date)) {
				return adjust(date - std::chrono::days(1), convention, cal);
			}
		}
	}

	enum class frequency {
		annually = 1,
		semiannually = 2,
		quarterly = 4,
		monthly = 12,
		weekly = 52,
	};

	// Work backwards from termination to effective in duration steps of period
	template<class Clock = std::chrono::system_clock, class Duration = std::chrono::system_clock::duration>
	struct periodic {
		std::chrono::time_point<Clock, Duration> effective, termination;
		Duration period;
	};

	template<class Clock, class Duration>
	class periodic_iterable {
		periodic<Clock, Duration> i;
		std::chrono::time_point<Clock, Duration> current;
		// order and direction of period are compatible
		constexpr bool valid() const
		{
			if (i.effective == i.termination) {
				return i.period.count() == 0;
			}

			return (i.effective < i.termination) xor (i.period.count() > 0);
		}
	public:
		using value_type = std::chrono::time_point<Clock, Duration>;

		// Work backward from termination.
		constexpr periodic_iterable(const periodic<Clock, Duration>& i)
			: i{ i }, current{ i.termination }
		{
			if (valid()) {
				while (current > i.effective) {
					auto current_ = current - i.period;
					if (current_ > i.effective) {
						current = current_;
					}
				}
			}
		}
		constexpr periodic_iterable(const periodic_iterable&) = default;
		constexpr periodic_iterable& operator=(const periodic_iterable&) = default;
		constexpr ~periodic_iterable() = default;

		constexpr bool operator==(const periodic_iterable&) const = default;

		constexpr auto begin() const
		{
			return *this;
		}
		constexpr auto end() const
		{
			return periodic_iterable(periodic(i.termination, i.termination, i.period));
		}

		constexpr explicit operator bool() const
		{
			return current != i.termination;
		}
		constexpr value_type operator*() const
		{
			return current;
		}
		constexpr periodic_iterable& operator++()
		{
			if (*this) {
				current += i.period;
			}

			return this;
		}
#ifdef _DEBUG
		static int test()
		{
			using year = std::chrono::year;
			{
				constexpr auto eff = year(2023) / 1 / 2;
				constexpr auto ter = year(2025) / 1 / 2;
				constexpr periodic<> p = { days(eff), days(ter), std::chrono::years(1) };
				constexpr auto pi = periodic_iterable(p);
				//static_assert(pi);
				//constexpr periodic_iterable pi2{ pi };
				//static_assert(pi2 == pi);
				//static_assert(days(eff) == *pi);
			}

			return 0;
		}
#endif // _DEBUG
	};

#ifdef _DEBUG
	int test() 
	{
		using year = std::chrono::year;
		using days = std::chrono::sys_days;
		using seconds = std::chrono::seconds;
		{
			constexpr auto d0 = year{ 2023 } / 1 / 1;
			constexpr auto d1 = year{ 2023 } / 1 / 2;
			constexpr auto dt = days(d1) - days(d0);
			constexpr auto dy = years(dt);
			static_assert(0 != dy.count());
			static_assert(1 == dt.count());
			constexpr auto y0 = dcf_years(d0, d1);
			static_assert(dy == y0);
		}

		return 0;
	}
#endif // _DEBUG

} // namespace fms::date