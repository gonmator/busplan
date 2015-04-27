#pragma once

#include <cassert>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "literal.hpp"

namespace Utility {

	class InvalidIniDoc: public std::runtime_error {
	public:
		InvalidIniDoc(const std::string& msg): std::runtime_error(msg) {}
	};

	class IniDoc {
	public:
		using Value = Literal;
		class Property {
		public:
			using Cont_ = std::vector < Literal > ;
			using value_type = Cont_::value_type;
			using size_type = Cont_::size_type;
			using reference = Cont_::reference;
			using const_reference = Cont_::const_reference;
			using iterator = Cont_::iterator;
			using const_iterator = Cont_::const_iterator;

			Property(): str_{}, items_{}, strSync_{true}, itemsSync_{true} {}
			Property(const Property& other): 
				str_{other.str_}, items_{other.items_}, strSync_{other.strSync_}, itemsSync_{other.itemsSync_} {

			}
			Property(Property&& other): 
				str_{std::move(other.str_)}, 
				items_{std::move(other.items_)}, 
				strSync_{other.strSync_}, 
				itemsSync_{other.itemsSync_} {

				assert(strSync_ || itemsSync_);
				other.strSync_ = other.itemsSync_ = true;
			}
			Property(std::string line): str_{std::move(line)}, items_{}, strSync_{true}, itemsSync_{false} {
			}
			template <typename InputIt> Property(InputIt first, InputIt last) : 
				str_{}, items_(first, last), strSync_{false}, itemsSync_{true} {
			}
			Property& operator=(const Property& other) {
				Property	tmp{other};

				swap(tmp);
				return *this;
			}
			Property& operator=(Property&& other) {
				str_ = std::move(other.str_);
				items_ = std::move(other.items_);
				strSync_ = other.strSync_;
				itemsSync_ = other.itemsSync_;
				other.strSync_ = other.itemsSync_ = true;
				return *this;
			}
			void assign(const std::string& line) {
				Property	tmp{line};
				swap(tmp);
			}
			template <typename InputIt> void assign(InputIt first, InputIt last) {
				Property	tmp{first, last};
				swap(tmp);
			}

			// interface access
			std::string& string() {
				syncStr();
				itemsSync_ = false;
				return str_;
			}
			const std::string& string() const {
				syncStr();
				return str_;
			}
			Cont_& items() {
				syncItems();
				strSync_ = false;
				return items_;
			}
			const Cont_& items() const {
				syncItems();
				return items_;
			}

		private:
			void swap(Property& other) {
				Property	tmp{std::move(other)};
				other = std::move(*this);
				*this = std::move(tmp);
			}

			void syncStr() const;
			void syncItems() const;

			mutable std::string	str_;
			mutable Cont_		items_;
			mutable bool		strSync_;
			mutable bool		itemsSync_;
		};
		using PropertyName = std::string;
		using Section = std::map < PropertyName, Property > ;
		using SectionName = std::string;
		using Doc = std::map < SectionName, Section > ;
        enum class DuplicateAction {
            doNothing,
            overwrite,
            fail
        };
		void clear() {
			doc_.clear();
		}
		Doc& doc() {
			return doc_;
		}
		const Doc& doc() const {
			return doc_;
		}

		void swap(Property& lhs, Property& rhs);
        void merge(const IniDoc& toMerge, DuplicateAction daction = DuplicateAction::doNothing);

	private:
		Doc	doc_;
	};

	std::istream& read(std::istream& is, IniDoc& inidoc);
	std::ostream& write(std::ostream& os, const IniDoc& inidoc);

	inline std::istream& operator>>(std::istream& is, IniDoc& inidoc) {
		return read(is, inidoc);
	}
	inline std::ostream& operator<<(std::ostream& os, const IniDoc& inidoc) {
		return write(os, inidoc);
	}

	template <typename T>
	bool get(const IniDoc& doc, const std::string& sectionName, const std::string& propertyName, T& value) {
		try {
			Literal	propL{doc.doc().at(sectionName).at(propertyName).string()};
			value = getValue<T>(propL);
		} catch (const std::exception&) {
			return false;
		}

		return true;
	}

} // Utility
