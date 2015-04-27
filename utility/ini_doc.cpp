#include <algorithm>
#include <utility>

#include "ini_doc.hpp"


namespace Utility {

	namespace {

		std::string& trim(std::string& str) {
			auto	pos = str.find_first_not_of(" \t");
			str.erase(0, pos);
			pos = str.find_last_not_of(" \t");
			if (pos != str.npos) {
				str.erase(pos + 1);
			}
			return str;
		}

		IniDoc::Section::value_type parseParameter(std::string& line) {
			auto	pos = line.find('=');
			if (pos == line.npos) {
				throw InvalidIniDoc(std::string("Invalid parameter: \"").append(line).append("\""));
			}

			std::string			parameterName = line.substr(0, pos);
			IniDoc::Property	parameterValue{line.substr(pos + 1)};

			return std::make_pair(std::move(parameterName), std::move(parameterValue));
		}

		void writeSection(std::ostream& os, const IniDoc::Section& section) {
			for (const auto& propertyPair : section) {
				os << propertyPair.first << "=" << propertyPair.second.string() << std::endl;
			}
			os << std::endl;
		}
	}	//

	void IniDoc::Property::syncStr() const {
		if (strSync_) return;
		
		str_.clear();
		if (!items_.empty()) {
			auto	itbeg = items_.cbegin();
			str_.append(getValue<std::string>(*itbeg++));
			std::for_each(itbeg, items_.cend(), [this](const value_type& item){
				str_.append(1, ',').append(item);
			});
		}
		strSync_ = true;
	}

	void IniDoc::Property::syncItems() const {
		if (itemsSync_) return;

		Cont_			value;
		auto			itbeg = str_.cbegin();
		auto			itend = str_.cend();
		auto			it0 = itbeg;
		decltype(it0)	it1;
		items_.clear();
		do {
			it1 = std::find(it0, itend, ',');
			items_.emplace_back(it0, it1);
			it0 = it1;
		} while (it1 != itend && ++it0 != itbeg);
		itemsSync_ = true;
	}

    void IniDoc::merge(const IniDoc &toMerge, DuplicateAction daction) {
        const auto& rdoc = toMerge.doc();
        for (const auto& rs: rdoc) {
            auto&   s = doc_[rs.first];
            for (const auto& rp: rs.second) {
                if (!s.count(rp.first) || daction == DuplicateAction::overwrite) {
                    s[rp.first] = rp.second;
                } else if (daction == DuplicateAction::fail) {
                    throw std::runtime_error(std::string{
                        "duplicated property: \""}.append(rp.first).append("\" in section \"").append(rp.first));
                }
            }
        }
    }

	std::istream& read(std::istream& is, IniDoc& inidoc) {
		auto&				doc = inidoc.doc();
		IniDoc::SectionName	sname{};

        doc[sname];

		for (std::string line; std::getline(is, line);) {
			//	trim line
			trim(line);

			//	skip empty and comments
			if (line.empty() || line[0] == ';') {
				continue;
			}

			if (*line.cbegin() == '[') {
				if (*line.crbegin() != ']') {
					throw InvalidIniDoc(std::string("Invalid line: \"").append(line).append("\""));
				}

                doc[sname = line.substr(1, line.size() - 2)];
				continue;
			}

			doc[sname].insert(parseParameter(line));
		}

		return is;
	}

	std::ostream& write(std::ostream& os, const IniDoc& inidoc) {
		const auto&	doc = inidoc.doc();
		if (doc.empty()) {
			return os;
		}

		auto	it = doc.cbegin();
		if (it->first.empty()) {
			//	default section
			writeSection(os, it->second);
			++it;
		}
		std::for_each(it, doc.cend(), [&os](const IniDoc::Doc::value_type& sectionPair) {
			os << "[" << sectionPair.first << "]" << std::endl;
			writeSection(os, sectionPair.second);
		});

		return os;
	}

} // Utility
