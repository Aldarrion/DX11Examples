#pragma once
#include <exception>
#include <string>

namespace Exception {

class InvalidCharacterException : public std::exception {
public:
    explicit InvalidCharacterException(char c)
            : exception((std::string("Character ") + c + " is not supported by this font.").c_str()) {
    }
};
}
