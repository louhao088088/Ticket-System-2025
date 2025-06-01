#pragma once
#include <cstddef>
#include <cstring>
#include <ostream>

namespace SJTU {
template <size_t string_length> class String {
  private:
    char data[string_length + 1] = {'\0'};

  public:
    String() = default;

    String(const char *str) {
        size_t i;
        for (i = 0; i < string_length && str[i] != '\0'; i++) {
            data[i] = str[i];
        }
        data[string_length] = '\0';
    }

    String(std::string &s) {
        size_t i;
        for (i = 0; i < s.size(); i++) {
            data[i] = s[i];
        }
        for (size_t j = i; j < string_length; j++) {
            data[j] = '\0';
        }
        data[string_length] = '\0';
    }

    char &operator[](size_t index) { return data[index]; }

    const char &operator[](size_t index) const { return data[index]; }

    const char *c_str() const { return data; }

    constexpr size_t length() const { return string_length; }

    constexpr size_t capacity() const { return string_length + 1; }

    bool operator==(const String &other) const {
        return strncmp(data, other.data, string_length) == 0;
    }

    bool operator!=(const String &other) const { return !(*this == other); }

    bool operator<(const String &other) const {
        return strncmp(data, other.data, string_length) < 0;
    }

    bool operator<=(const String &other) const {
        return strncmp(data, other.data, string_length) <= 0;
    }

    bool operator>(const String &other) const {
        return strncmp(data, other.data, string_length) > 0;
    }

    bool operator>=(const String &other) const {
        return strncmp(data, other.data, string_length) >= 0;
    }

    friend std::ostream &operator<<(std::ostream &os, const String &str) {
        os << str.data;
        return os;
    }

    String &operator=(const String &other) {
        if (this != &other) {
            strncpy(data, other.data, string_length);
            data[string_length] = '\0';
        }
        return *this;
    }
};
} // namespace SJTU