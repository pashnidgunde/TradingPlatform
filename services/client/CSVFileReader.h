#pragma once

#include <string>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>
#include <string_view>


namespace client {

    class CSVFileReader {
    public:
        using OrderInstructions = std::vector<std::string>;

        explicit CSVFileReader(const std::string &filename, const char commentToken = '#') :
                _commentToken(commentToken) {
            if (!try_read(filename)) {
                throw std::runtime_error("Couldn't read the file");
            }
        }

        [[nodiscard]] const OrderInstructions instructions() const {
            return _instructions;
        }

    private:
        [[nodiscard]] bool isComment(const std::string_view &line) const {
            return line[0] == _commentToken;
        }

        bool try_read(const std::string &file_name) {
            std::fstream fs;
            fs.open(file_name, std::ios::in);
            if (!fs.is_open()) return false;

            std::string line;
            while (getline(fs, line)) {
                if (!isComment(line)) {
                    _instructions.emplace_back(std::move(line));
                }
            }

            fs.close();
            return true;
        }

    private:
        const char _commentToken = '#';
        OrderInstructions _instructions;
    };

}


