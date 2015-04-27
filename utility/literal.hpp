#pragma once

#include <algorithm>
#include <string>
#include <type_traits>
#include <vector>

namespace Utility {

	namespace {
		template <typename Cond1, typename Cond2>
		struct And: std::integral_constant < bool, Cond1::value && Cond2::value > {};

		template <typename Cond1, typename Cond2>
		struct Or: std::integral_constant < bool, Cond1::value || Cond2::value > {};

		template <typename Condition, typename T = void>
		using EnableIf = typename std::enable_if<Condition::value, T>::type;

		template <typename Condition, typename T = void>
		using DisableIf = typename std::enable_if<!Condition::value, T>::type;
	}

    class Literal;

	template <typename T>
	inline T getValue(const Literal&) {
        throw std::bad_cast{};
    }

	//	A literal is a string, but a string is not necessary a literal
	class Literal: public std::string {
	public:
		typedef std::string				string_type;
		typedef string_type::size_type	size_type;
		typedef string_type::value_type	char_type;
        typedef std::vector<Literal>    literal_list;

		Literal(): string_type{} {}
		Literal(
			size_type	count,
			char_type	ch): string_type(count, ch) {}
		Literal(
			const string_type&	str,
			size_type			pos,
			size_type			count = string_type::npos): string_type(str, pos, count) {}
		Literal(
			const char_type*	s,
			size_type			count): string_type(s, count) {}
		Literal(
			const char_type*	s): string_type(s) {}
		template <class InputIt>
		Literal(
			InputIt	first,
			InputIt	last) : string_type(first, last) {}
		Literal(
			const Literal&	other): string_type(static_cast<const string_type&>(other)) {}
		Literal(
			const string_type&	other): string_type(other) {}
		Literal(
			Literal&& other): string_type(std::move(static_cast<string_type>(other))) {}
		Literal(
			string_type&& other): string_type(std::move((other))) {}
		Literal(
			std::initializer_list<char_type>	init): string_type(init) {}

		Literal& operator=(const Literal& l) {
			*static_cast<string_type*>(this) = l;
			return *this;
		}
		Literal& operator=(Literal&& l) {
			*static_cast<string_type*>(this) = std::move(l);
			return *this;
		}

		template <typename T>
		T as() const {
            return getValue<T>(*this);
		}

		bool isANumber() const {
			return isDecimal() || isHexadecimal() || isReal();
		}
		bool isDecimal() const {
			if (size() < 1) {
				return false;
			}
			size_type	pos = 0;
			if (operator[](0) == '+' || operator[](0) == '-') {
				++pos;
			}
			if (size() <= pos) {
				return false;
			}
			return std::find_if_not(cbegin() + pos, cend(), [](char_type c) { 
				return c >= '0' && c <= '9'; 
			}) == end();
		}
		bool isHexadecimal() const {
			if (size() < 3) {
				return false;
			}
			if (operator[](0) != '0' || operator[](1) != 'x') {
				return false;
			}
			return std::find_if_not(cbegin() + 2, cend(), [](char_type c) {
                return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
			}) == end();
		}
		bool isReal() const {
			if (size() < 1) {
				return false;
			}
			size_type	pos = 0;
			if (operator[](0) == '+' || operator[](0) == '-') {
				++pos;
			}
			if (size() <= pos) {
				return false;
			}
			auto		dots = std::count(cbegin() + pos, cend(), '.');
			if (dots > 1) {
				return false;
			}
			auto		digits = std::count_if(cbegin() + pos, cend(), [](char_type c) {
				return c >= '0' && c <= '9';
			});
            return digits + dots == static_cast<std::make_signed<size_type>::type>(size() - pos);
		}

		template <typename T, typename = std::enable_if<std::is_floating_point<T>::value>>
		T asReal() const {
			auto	it = cbegin();
			auto	itend = cend();
			if (it == itend) {
				throw std::bad_cast{};
			}

			T	sign = extractSign<T>(it);
			if (it == itend) {
				throw std::bad_cast{};
			}

			auto	dotit = std::find(it, itend, '.');
			auto	lv = extractDecimal<T>(it, dotit);
			
			auto	rv = 0.0;
			if (dotit != itend) {
				++dotit;
				rv = extractDecimal<T>(dotit, itend);
				for (auto i = itend - dotit; i > 0; --i) {
					rv /= 10.0;
				}
			}

			return sign * (lv + rv);
		}
		//double asReal() const {
		//	auto	it = cbegin();
		//	if (it == cend()) {
  //              // not C++ compliant: throw std::bad_cast("literal is not a real number");
  //              throw std::bad_cast{};
		//	}

		//	double	sign = 1.0;
		//	double	lv = 0.0;
		//	double	rv = 0.0;
		//	switch (operator[](0)) {
		//		case '-':
		//			sign = -1.0;
		//		case '+':
		//			++it;
		//	}
		//	if (it == cend()) {
  //              //  not C++ compliant: throw std::bad_cast("literal is not a real number");
  //              throw std::bad_cast{};
		//	}

		//	auto	dotit = std::find(it, cend(), '.');
		//	std::for_each(it, dotit, [&lv](char c) {
		//		short	v = c - '0';
		//		if (v < 0 || v > 9) {
  //                  // not C++ compliant: throw std::bad_cast("literal is not a real number");
  //                  throw std::bad_cast{};
		//		}
		//		lv *= 10;
		//		lv += v;
		//	});

		//	if (dotit != cend()) {
		//		std::for_each(crbegin(), const_reverse_iterator(++dotit), [&rv](char c) {
		//			double	v = c - '0';
		//			if (v < 0 || v > 9) {
  //                      // not C++ compliant: throw std::bad_cast("literal is not a real number");
  //                      throw std::bad_cast{};
		//			}
		//			rv /= 10;
		//			rv += v / 10;
		//		});
		//	}

		//	return sign * ( lv + rv );
		//}

		template <typename T, typename = std::enable_if<std::is_integral<T>::value>>
		T asDecimal() const {
			auto	it = cbegin();
			if (it == cend()) {
				throw std::bad_cast{};
			}

			T	sign = extractSign<T>(it);
			if (it == cend()) {
				throw std::bad_cast{};
			}

			return sign * extractDecimal<T>(it, cend());
		}

        literal_list asList() const {
            literal_list    rv;
            auto			itbeg = cbegin();
            auto			itend = cend();
            auto			it0 = itbeg;
            decltype(it0)	it1;
            do {
                it1 = std::find(it0, itend, ',');
                rv.emplace_back(it0, it1);
                it0 = it1;
            } while (it1 != itend && ++it0 != itbeg);

            return rv;
        }

	private:
		template <typename T>
		static T extractSign(string_type::const_iterator& it) {
			T	sign = 1;
			switch (*it) {
			case '-':
				sign = -1;
			case '+':
				++it;
			}

			return sign;
		}

		template <typename T>
		static T extractDecimal(string_type::const_iterator beg, string_type::const_iterator end) {
			T	rv = 0;

			std::for_each(beg, end, [&rv](char c) {
				short	v = c - '0';
				if (v < 0 || v > 9) {
					throw std::bad_cast{};
				}
				rv *= 10;
				rv += v;
			});

			return rv;
		}
	};

    using LiteralList = Literal::literal_list;

	inline bool isANumber(
		const Literal& l) {
		
		return l.isANumber();
	}

	inline bool isDecimal(
		const Literal&	l) {

		return l.isDecimal();
	}
	
	inline bool isHexadecimal(
		const Literal&	l) {

		return l.isHexadecimal();
	}
	inline bool isReal(
		const Literal&	l) {

		return l.isReal();
	}

    //  getValue
    //  --------
    template <>
    inline Literal getValue(const Literal &l) {
        return l;
    }
    template <>
    inline Literal::string_type getValue(const Literal &l) {
        return static_cast<const Literal::string_type&>(l);
    }
    template <>
	inline const char* getValue(const Literal &l) {
        return l.c_str();
    }
	template <>
	inline int getValue(const Literal &l) {
		return l.asDecimal<int>();
	}
	template <>
	inline unsigned getValue(const Literal &l) {
		return l.asDecimal<unsigned>();
	}
	template <>
	inline long getValue(const Literal &l) {
		return l.asDecimal<long>();
	}
	template <>
	inline long long getValue(const Literal &l) {
		return l.asDecimal<long long>();
	}
	template<>
	inline double getValue(const Literal &l) {
        return l.asReal<double>();
    }
    template<>
    inline LiteralList getValue(const Literal& l) {
        return l.asList();
    }


	template <typename T>
	Literal makeLiteral(T v);

	template <>
	inline Literal makeLiteral<std::string>(std::string v) {
		return Literal{std::move(v)};
	}
	template <>
	inline Literal makeLiteral<double>(double v) {
		return Literal{std::to_string(v)};
	}
	template <>
	inline Literal makeLiteral<unsigned int>(unsigned int v) {
		return Literal{std::to_string(v)};
	}

} // Utility
