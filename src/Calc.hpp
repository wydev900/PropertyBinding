#pragma once

struct Plus {
    template <typename T1, typename T2>
    static inline auto calc(const T1& a, const T2& b) { return a + b; }
};

struct Minus {
    template <typename T1, typename T2>
    static inline auto calc(const T1& a, const T2& b) { return a - b; }
};

struct Multiplies {
    template <typename T1, typename T2>
    static inline auto calc(const T1& a, const T2& b) { return a * b; }
};

struct Divides {
    template <typename T1, typename T2>
    static inline auto calc(const T1& a, const T2& b) { return a / b; }
};

struct Modulus {
    template <typename T1, typename T2>
    static inline auto calc(const T1& a, const T2& b) { return a % b; }
};

struct Negate {
    template <typename T>
    static inline auto calc(const T& a) { return -a; }
};

struct Equal {
    template <typename T1, typename T2>
    static inline auto calc(const T1& a, const T2& b) { return a == b; }
};

struct NotEqual {
    template <typename T1, typename T2>
    static inline auto calc(const T1& a, const T2& b) { return a != b; }
};

struct Greater {
    template <typename T1, typename T2>
    static inline auto calc(const T1& a, const T2& b) { return a > b; }
};

struct Less {
    template <typename T1, typename T2>
    static inline auto calc(const T1& a, const T2& b) { return a < b; }
};

struct GreaterEqual {
    template <typename T1, typename T2>
    static inline auto calc(const T1& a, const T2& b) { return a >= b; }
};

struct LessEqual {
    template <typename T1, typename T2>
    static inline auto calc(const T1& a, const T2& b) { return a <= b; }
};

struct LogicalAnd {
    template <typename T1, typename T2>
    static inline auto calc(const T1& a, const T2& b) { return a && b; }
};

struct LogicalOr {
    template <typename T1, typename T2>
    static inline auto calc(const T1& a, const T2& b) { return a || b; }
};

struct LogicalNot {
    template <typename T>
    static inline auto calc(const T& a) { return !a; }
};

struct BitAnd {
    template <typename T1, typename T2>
    static inline auto calc(const T1& a, const T2& b) { return a & b; }
};

struct BitOr {
    template <typename T1, typename T2>
    static inline auto calc(const T1& a, const T2& b) { return a | b; }
};

struct BitXor {
    template <typename T1, typename T2>
    static inline auto calc(const T1& a, const T2& b) { return a ^ b; }
};

struct BitNot {
    template <typename T>
    static inline auto calc(const T& a) { return ~a; }
};

// Revers args
template <class O>
struct Revers {
    template <typename T1, typename T2>
    static inline auto calc(const T1& a, const T2& b) { return O::calc(b, a); }
};

struct Noop {
    template <typename T>
    static inline auto calc(const T& a) { return a; }
};
