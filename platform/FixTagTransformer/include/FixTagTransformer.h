#pragma once

#define BOOST_SPIRIT_X3_DEBUG

#include <string>
#include <utility>
#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/core/parse.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

#include <iostream>
#include <list>
#include <numeric>
#include <string>

namespace x3 = boost::spirit::x3;

// namespace ast {
//     struct printer {
//         void operator() (std::pair <uint32_t, const std::string> result) {
//             std::cout << result.first << result.second;
//         }
//     };
// }
//
// namespace grammar {
//
//     x3::rule<class statement, std::pair <uint32_t, const std::string>> const
//     statement("statement"); BOOST_SPIRIT_DEFINE(
//         statement
//     );
//
//     auto ftt = statement;
// }
//
//
// struct eval {
//
//
//
// };

struct Statement {
  explicit Statement(std::string message) : _message(std::move(message)) {}

  std::string _message;

  [[nodiscard]] bool apply(const std::string& rule) {
    using boost::spirit::x3::ascii::space_type;
    using x3::_attr;
    using x3::alnum;
    using x3::char_;
    using x3::uint_;
    space_type space;

    const auto begin = _message.begin();

    auto tag_start = std::string::npos;
    auto value_start = std::string::npos;
    auto value_end = std::string::npos;
    auto tag_end = std::string::npos;

    auto tag = [&](auto& tag) {
      const auto& tag_number = _attr(tag);
      tag_start = _message.find(tag_number);
      if (tag_start == std::string::npos) {
        tag_start = _message.size();
        _message += tag_number + '=';
        return;
      }
      value_start = tag_start + tag_number.size() + 1;
      value_end = _message.find_first_of(';', value_start);
      tag_end = value_end + 1;
    };

    auto value = [&](auto& value) {
      const auto& tag_value = _attr(value);
      // std::cout << "Value :" << _attr(value) << typeid(_attr(value)).name()
      // << "\n";
      if (_message.back() == '=') {
        if (!tag_value.empty()) {
          _message += tag_value + ";";
        } else {
          _message.erase(begin + tag_start, _message.end());
        }
        return;
      }

      if (tag_value.empty()) {
        _message.erase(begin + tag_start, begin + tag_end);
      } else {
        _message.replace(begin + value_start, begin + value_end, tag_value);
      }
    };

    auto statement_rule =
        (+(char_ - '='))[tag] >> '=' >> (*(char_ - ';'))[value] >> ';';
    // auto statement = ((+(char_ - '=')) >> char_('='))[tag] >> (+(char_ -
    // ';'))[value] >> ';';

    auto rule_begin = rule.begin();
    auto rule_end = rule.end();
    bool r = x3::phrase_parse(rule_begin, rule_end, statement_rule, space);

    if (rule_begin != rule_end)  // fail if we did not get a full match
      return false;
    return r;
  }

  [[nodiscard]] const std::string& applied() const { return _message; }
};

// struct FixTagTransformer {
//	FixTagTransformer() = delete;
//	explicit FixTagTransformer(const std::string& _message, const
// std::vector<std::string>& rules) :
//         _statement(_message),
//		_rules(rules)
//	{}
//
//     Statement _statement;
//	std::vector<std::string> _rules;
//
////	bool apply() {
////        for (const auto& rule : _rules) {
////            if  (!_statement.apply(rule)) {
////              std::cout << "Failed to apply rule " << rule;
////              break;
////            }
////            std::cout << "Applied rule : " << _statement.applied();
////        }
////        return true;
////	}
//};
