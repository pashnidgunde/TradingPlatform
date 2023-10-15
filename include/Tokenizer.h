#pragma once

#include <string>
#include <vector>
#include <regex>

namespace platform::util {
    struct Tokenizer {
        static const std::vector<std::string> tokenize(const std::string &str) {
            const std::regex re(R"([\s|,]+)");
            std::sregex_token_iterator it{str.begin(),
                                          str.end(), re, -1};
            std::vector<std::string> tokenized{it, {}};

            // Additional check to remove empty strings
            tokenized.erase(
                    std::remove_if(tokenized.begin(), tokenized.end(), [](std::string const &s) {
                        return s.size() == 0;
                    }),
                    tokenized.end());

            return tokenized;
        }

    };
}