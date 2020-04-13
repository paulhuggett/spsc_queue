#ifndef EMPTY_VALUE_HPP
#define EMPTY_VALUE_HPP

#include <limits>

template <typename T>
struct empty_value {};
template <typename T>
struct empty_value<T *> {
    static constexpr T * value = nullptr;
};

template <>
struct empty_value<char> {
    static constexpr auto value = std::numeric_limits<char>::min ();
};
constexpr char empty_value<char>::value;

template <>
struct empty_value<int> {
    static constexpr auto value = std::numeric_limits<int>::min ();
};
constexpr int empty_value<int>::value;

template <>
struct empty_value<long> {
    static constexpr auto value = std::numeric_limits<long>::min ();
};
constexpr long empty_value<long>::value;


template <>
struct empty_value<unsigned char> {
    static constexpr auto value = std::numeric_limits<unsigned char>::max ();
};
constexpr unsigned char empty_value<unsigned char>::value;

template <>
struct empty_value<unsigned int> {
    static constexpr auto value = std::numeric_limits<unsigned int>::max ();
};
constexpr unsigned int empty_value<unsigned int>::value;

template <>
struct empty_value<unsigned long> {
    static constexpr auto value = std::numeric_limits<unsigned long>::max ();
};
constexpr unsigned long empty_value<unsigned long>::value;

#endif // EMPTY_VALUE_HPP
