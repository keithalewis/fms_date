// fms_date.h - Date and time calculation
#pragma once
#include <chrono>
#include <iterator>
#include <tuple>

namespace fms::date {

	// Calendar date.
	using ymd = std::chrono::year_month_day;

	constexpr ymd make_ymd(int y, unsigned int m, unsigned int d)
	{
		return ymd(std::chrono::year(y), std::chrono::month(m), std::chrono::day(d));
	}
	constexpr std::tuple<std::chrono::year, std::chrono::month, std::chrono::day> from_ymd(const ymd& d)
	{
		return { d.year(), d.month(), d.day() };
	}
	constexpr ymd ymd_error = make_ymd(0, 0u, 0u);

	// Construct sys_days time point from ymd.
	using days = std::chrono::sys_days;
	constexpr days make_days(int y, int m, int d)
	{
		// ymd has operator operator sys_days conversion
		return days(make_ymd(y, m, d));
	}

	// duration as double in years
	using years = std::chrono::duration<double, std::chrono::years::period>;

	// d0 - d1 in years. If t = d0 - d1 then d1 = d0 - t and d0 = d1 + t
	constexpr years operator-(const ymd& d0, const ymd& d1)
	{
		return years(days(d0) - days(d1));
	}

#ifdef _DEBUG
	int basic_date_test()
	{
		{
			constexpr auto de = make_ymd(0, 0, 0);
			static_assert(de == ymd_error);
			auto [ y,m,d ] = from_ymd(de);
			/*
			assert(0 == y);
			assert(0u == m);
			assert(0u == d);
			ymd d2(y, m, d);
			assert(d2 == de);
			*/
		}
		{
			constexpr auto d0 = make_days(2023, 4, 5);
			constexpr auto d1 = make_days(2024, 4, 5);
			constexpr auto dd = d0 - d1;
			static_assert(d1 + dd == d0);
			static_assert(d0 - dd == d1);
		}
		{
			constexpr auto d0 = make_days(2023, 4, 5);
			constexpr auto d1 = make_days(2024, 4, 6);
			constexpr auto dd = d0 - d1;
			static_assert(d1 + dd == d0);
			static_assert(d0 - dd == d1);
		}
		{
			constexpr auto d0 = make_days(2023, 4, 5);
			constexpr auto d1 = make_days(2024, 7, 6);
			constexpr auto dd = d0 - d1;
			static_assert(d1 + dd == d0);
			static_assert(d0 - dd == d1);
		}

		return 0;
	}
#endif // _DEBUG

	// Periodic times in [effective, termination] working backwards from termination in period steps.
	template<class Clock = std::chrono::system_clock, 
		class Duration = typename Clock::duration,
		class time_point = std::chrono::time_point<Clock, Duration>>
	class periodic {
		time_point effective, termination;
		Duration period;
	public:
		constexpr periodic(const time_point& effective, const time_point& termination, const Duration& period)
			: effective{ effective }, termination{ termination }, period{ period }
		{ }
		constexpr periodic(const periodic&) = default;
		constexpr periodic& operator=(const periodic&) = default;
		constexpr ~periodic() = default;

		constexpr bool operator==(const periodic&) const = default;

		class iterable {
			periodic<Clock, Duration> i;
			time_point current;

			constexpr void reset()
			{
				// don't reset end()
				if (current <= i.termination) {
					while (current - i.period > i.effective) {
						current = current - i.period;
					}
				}
			}
		public:
			// STL iterator
			using iterator_category = std::forward_iterator_tag;
			using value_type = ymd;

			constexpr iterable(const periodic<Clock, Duration>& i, const time_point& current)
				: i{ i }, current{ current }
			{ }
			// Work backward from termination.
			constexpr iterable(const periodic<Clock, Duration>& i)
				: iterable(i, i.termination)
			{
				if (valid()) {
					reset();
				}
			}
			constexpr iterable(const iterable&) = default;
			constexpr iterable& operator=(const iterable&) = default;
			constexpr ~iterable() = default;
 
			constexpr bool operator==(const iterable& i_) const = default;

			// order and direction of period are compatible
			constexpr bool valid() const
			{
				if (i.effective == i.termination) {
					return i.period.count() == 0;
				}

				return (i.effective < i.termination) == (i.period.count() > 0);
			}

			constexpr auto begin() const
			{
				return iterable(i);
			}
			constexpr auto end() const
			{
				iterable(i, termination + period);
			}

			constexpr explicit operator bool() const
			{
				return current <= i.termination;
			}
			constexpr value_type operator*() const
			{
				return value_type{ floor<std::chrono::days>(current) };
			}
			constexpr iterable& operator++()
			{
				if (*this) {
					current += i.period;
				}

				return *this;
			}
		};
#ifdef _DEBUG
		static int test()
		{
			using year = std::chrono::year;
			constexpr auto eff = year(2023) / 1 / 2;
			constexpr auto ter = year(2025) / 1 / 2;
			{
				constexpr periodic<> p(days(eff), days(ter), std::chrono::years(1));
				constexpr periodic::iterable pi(p);
				static_assert(pi);
				constexpr auto pi2{ pi };
				static_assert(pi2 == pi);
				constexpr auto pi3 = pi2;
				static_assert(!(pi3 != pi2));
				assert(eff == *pi3);

				iterable pi_{ pi };
				assert(pi_);
				assert(eff == *pi_);
				++pi_;
				assert(pi_);
				auto p2 = days(eff) + years(1);
				//ymd p3(p2);
				//assert(days(*pi_) == p2);
				++pi_;
				assert(ter == *pi_);
				++pi_;
				assert(!pi_);
			}

			return 0;
		}
#endif // _DEBUG
	};

	// Day count fraction appoximately equal to time in year between dates.
	using dcf_ = years(*)(const ymd&, const ymd&);
	namespace dcf {
		// Day count fraction in years from d0 to d1.
		constexpr years _years(const ymd& d0, const ymd& d1)
		{
			return d1 - d0;
		}
		constexpr years _30_360(const ymd& t0, const ymd& t1)
		{
			auto [y0, m0, d0] = from_ymd(t0);
			auto [y1, m1, d1] = from_ymd(t1);

			if (d0 == std::chrono::day(31)) {
				d0 = std::chrono::day(30);
			}
			if (d1 == std::chrono::day(31) and d0 > std::chrono::day(29)) {
				d1 = std::chrono::day(30);
			}

			return (360 * (y1 - y0) + 30*(m1 - m0) + (d1 - d0))/360.;
		}
	}

	// Return true if date is a holiday.
	using holiday = bool(*)(const ymd&);
	namespace holidays {

		// Every year holiday on month and day
		constexpr bool month_day(const ymd& d, const std::chrono::month& month, const std::chrono::day& day)
		{
			return (d.month() == month) and (d.day() == day);
		}

		constexpr bool new_year_day(const ymd& d)
		{
			return month_day(d, std::chrono::January, std::chrono::day(1));
		}

		// ...

		constexpr bool christmas_day(const ymd& d)
		{
			return month_day(d, std::chrono::December, std::chrono::day(25));
		}

	} // namespace holiday

	// Return true on non-trading days.
	using calendar = bool(*)(const ymd&);
	namespace calendars {

		constexpr bool weekday(const ymd& d)
		{
				auto w = std::chrono::year_month_weekday(d);
				auto wd = w.weekday();

				return (wd == std::chrono::Saturday) or (wd == std::chrono::Sunday);
		}
		constexpr bool example(const ymd& d)
		{
			return weekday(d) or holidays::new_year_day(d);
		}

		// constexpr bool nyse(const ymd& d) ...

	} // namespace calendars

	// Roll to business day conventions.
	enum class roll {
		none,
		following, // following business day
		previous,  // previous business day
		modified_following, // following business day unless different month
		modified_previous,  // previous business day unless different month
	};

	constexpr ymd adjust(const ymd& date, roll convention, const calendar& cal = calendars::weekday)
	{
		if (!cal(date)) {
			return date;
		}

		switch (convention) {
		case roll::none:
			return date;
		case roll::previous:
			return adjust(days(date) - std::chrono::days(1), convention, cal);
		case roll::following:
			return adjust(days(date) + std::chrono::days(1), convention, cal);
		case roll::modified_following:
		{
			const auto date_ = adjust(days(date), roll::following, cal);
			return date_.month() == date.month()
				? date_
				: adjust(date, roll::previous, cal);
		}
		case roll::modified_previous:
		{
			const auto date_ = adjust(days(date), roll::previous, cal);
			return date_.month() == date.month()
				? date_
				: adjust(date, roll::following, cal);
		}
		default:
			return ymd_error;
		}

		return date;
	}

	enum class frequency {
		annually = 1,
		semiannually = 2,
		quarterly = 4,
		monthly = 12,
	};
	constexpr auto tenor(frequency f)
	{
		return std::chrono::months(12 / (int)f);
	}

#ifdef _DEBUG
	int test() 
	{
		using year = std::chrono::year;
		using days = std::chrono::sys_days;
		//using seconds = std::chrono::seconds;
		{
			constexpr auto d0 = year{ 2023 } / 1 / 1;
			constexpr auto d1 = year{ 2023 } / 1 / 2;
			constexpr auto dt = days(d1) - days(d0);
			constexpr auto dy = years(dt);
			static_assert(0 != dy.count());
			static_assert(1 == dt.count());
			constexpr auto y0 = dcf::_years(d0, d1);
			static_assert(dy == y0);
		}

		return 0;
	}
#endif // _DEBUG

} // namespace fms::date
