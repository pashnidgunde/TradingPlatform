#pragma once

#define BOOST_SPIRIT_X3_DEBUG

#include <string>
#include <vector>

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <string>
#include <list>
#include <numeric>
#include <iostream>

//namespace client {
//    namespace ast {
//        struct program {
//
//        };
//
//        struct printer {
//
//        };
//
//        struct eval {
//
//        };
//    }
//
//
//}
//
//namespace client {
//    namespace ftt_grammer {
//
//    }
//}


struct FixTagTransformer {
	FixTagTransformer() = delete;
	explicit FixTagTransformer(const std::vector<std::string>& rules) :
		_rules(rules)
	{}

	std::vector<std::string> _rules;

	static bool transform(const std::string& str) {
        
        /*using iterator_type = std::string::const_iterator ;
        using ast_program = client::ast::program;
        using ast_print = client::ast::printer;
        using ast_eval = client::ast::eval;*/

        //auto& ftt = client::ftt;           // Our grammar
        //ast_program program;                // Our program (AST)
        //ast_print print;                    // Prints the program
        //ast_eval eval;                      // Evaluates the program
        
        namespace x3 = boost::spirit::x3;
        using x3::uint_;
        using x3::alnum;
        using boost::spirit::x3::ascii::space_type;

        auto statement = (+(uint_)) >> '=' >> *alnum >> ';';
        //auto statement = (+(char_)-'=') >> '=';
        //auto statement = (+(char_)-'=') >> '=' >> (+(char_)-';') >> ';';
        //auto tag_values = (statement);

        auto begin = str.begin();
        auto end = str.end();
        space_type space;
        bool r = phrase_parse(begin, end, statement, space);
        return r && begin == end;
	}
};
