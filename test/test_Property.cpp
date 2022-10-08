#include "gtest/gtest.h"
#include <iterator>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>
#include "../src/Property.hpp"

TEST(Property, int) {
    int A = 1;
    int B = 2;
    int C = 3;
    int D = 4;
    bool M_BOOL = false;

    property<int> a = A;
    property<int> b = B;
    property<int> c = C;
    property<bool> d = 0;
    EXPECT_EQ(d.value(), false);
    d = D;
    EXPECT_EQ(d.value(), true);

    auto bool_a3 = (a == 3);
    EXPECT_EQ(bool_a3.value(), false);

    property<int> aaa = a;
    property<int> bbb = b;

    property<bool> bool_false = M_BOOL;

    aaa = bool_a3;
    d = (a + b);

    auto test = [&] {

        EXPECT_EQ(a.value(), A);
        EXPECT_EQ(b.value(), B);
        EXPECT_EQ(c.value(), C);

        auto bool_true = !bool_false;
        EXPECT_EQ(bool_true.value(), true);

        auto true_add_3 = bool_true + 3;
        EXPECT_EQ(true_add_3.value(), 4);

        auto bool_add_num = bool_false + 3;
        EXPECT_EQ(bool_add_num.value(), 3);

        auto AB = A + B;
        auto ABC = AB + C;
        auto ABC5 = ABC + 5;
        auto notAB = !AB;

        auto ab = a + b;
        EXPECT_EQ(ab.value(), AB);

        auto abc = ab + c;
        auto cab = c + ab;
        EXPECT_EQ(abc.value(), ABC);
        EXPECT_EQ(cab.value(), ABC);

        auto abc_cab = abc + cab;
        EXPECT_EQ(abc_cab.value(), ABC + ABC);

        auto abc5 = abc + 5;
        auto _5abc = 5 + abc;
        EXPECT_EQ(abc5.value(), ABC5);
        EXPECT_EQ(_5abc.value(), ABC5);

        auto abc_5 = (a + b + c) + 5;
        auto _5_abc = 5 + (a + b + c);
        EXPECT_EQ(abc_5.value(), ABC5);
        EXPECT_EQ(_5_abc.value(), ABC5);

        auto a_b_c = a + b + c;
        EXPECT_EQ(a_b_c.value(), ABC);

        auto _AB = -AB;
        auto _ab = -ab;
        auto __ab = -(a + b);
        EXPECT_EQ(_ab.value(), _AB);
        EXPECT_EQ(__ab.value(), _AB);

        auto ab_copy = !ab;
        auto ab_move = !(a + b);
        EXPECT_EQ(ab_copy.value(), notAB);
        EXPECT_EQ(ab_move.value(), notAB);

        property<int> x = abc;
        property<int> xx = a + b + c;
        EXPECT_EQ(x.value(), ABC);
        EXPECT_EQ(xx.value(), ABC);

        EXPECT_EQ((a == 1).value(), A == 1);
        EXPECT_EQ((a != 1).value(), A != 1);
        EXPECT_EQ((a > 0).value(), A > 0);
        EXPECT_EQ((a > 1).value(), A > 1);
        EXPECT_EQ((a < 1).value(), A < 1);
        EXPECT_EQ((a < 2).value(), A < 2);
        EXPECT_EQ((a <= 1).value(), A <= 1);
        EXPECT_EQ((a >= 1).value(), A >= 1);
        EXPECT_EQ((a && b).value(), A && B);
        EXPECT_EQ((!a && b).value(), !A && B);
        EXPECT_EQ((a || b).value(), A || B);
        EXPECT_EQ((!a || !b).value(), !A || !B);
        EXPECT_EQ((!a).value(), !A);
        EXPECT_EQ((a & b).value(), A & B);
        EXPECT_EQ((a | b).value(), A | B);
        EXPECT_EQ((a ^ b).value(), A ^ B);
        EXPECT_EQ((~a).value(), ~A);
    };

    int newValue = 0;
    a.onValueChanged([&newValue](int v) {
        newValue = v;
        std::cout << v << " <--------------onValueChanged" << std::endl;
    });
    EXPECT_EQ(newValue, 0);

    test();
    a = 5;
    EXPECT_EQ(newValue, 5);
    A = 5;
    test();
}

TEST(Property, test1) {
    int A = 0;
    property<int> a = A;
    property<int> b = a;
    property<bool> c = b;
    property<bool> d = !b;
    property<bool> e = a + b;

    auto dd = a + b;
    property<bool> f = dd;

    property<bool> g = a || 0;
    property<bool> h = a && 3;
    property<bool> i = d || f;

    int aValue = a.value();
    int bValue = b.value();
    bool cValue = c.value();
    bool dValue = d.value();
    bool eValue = e.value();
    bool fValue = f.value();
    bool gValue = g.value();
    bool hValue = h.value();
    bool iValue = i.value();

    a.onValueChanged([&aValue](auto v) {
        aValue = v;
    });

    b.onValueChanged([&bValue](auto v) {
        bValue = v;
    });

    c.onValueChanged([&cValue](auto v) {
        cValue = v;
    });

    d.onValueChanged([&dValue](auto v) {
        dValue = v;
    });

    e.onValueChanged([&eValue](auto v) {
        eValue = v;
    });

    f.onValueChanged([&fValue](auto v) {
        fValue = v;
    });

    g.onValueChanged([&gValue](auto v) {
        gValue = v;
    });

    h.onValueChanged([&hValue](auto v) {
        hValue = v;
    });

    i.onValueChanged([&iValue](auto v) {
        iValue = v;
    });

    // bool zValue = f;
    // d.onValueChanged([&zValue](auto v) {
    //     zValue = v;
    // });

    EXPECT_EQ(aValue, 0);
    EXPECT_EQ(bValue, 0);
    EXPECT_EQ(cValue, false);
    EXPECT_EQ(dValue, true);
    EXPECT_EQ(eValue, false);
    EXPECT_EQ(fValue, false);
    EXPECT_EQ(gValue, false);
    EXPECT_EQ(hValue, false);
    EXPECT_EQ(iValue, true);
    a = 8;
    EXPECT_EQ(aValue, 8);
    EXPECT_EQ(bValue, 8);
    EXPECT_EQ(cValue, true);
    EXPECT_EQ(dValue, false);
    EXPECT_EQ(eValue, true);
    EXPECT_EQ(fValue, true);
    EXPECT_EQ(gValue, true);
    EXPECT_EQ(hValue, true);
    EXPECT_EQ(iValue, true);
}

#include "../src/Item.h"

TEST(Property, test2){
    Rectangle rect{
        .x = 5,
        .y = 5,
        .width = 100,
        .height = rect.width,
    };
    EXPECT_EQ(rect.width.value(), 100);
    EXPECT_EQ(rect.height.value(), 100);
    rect.width = 88;
    EXPECT_EQ(rect.width.value(), 88);
    EXPECT_EQ(rect.height.value(), 88);
}

TEST(Property, addr) {
    readonly<int> a = 3;
    BasicProperty<int, false>* pa = &a;
    void* pb = &a;
    EXPECT_EQ(pa, pb);
}

TEST(Property, readonly) {
    readonly<int> a = 3;
    EXPECT_EQ(a.value(), 3);
    property<int> b = a;
    EXPECT_EQ(b.value(), 3);
    property<bool> c = a;
    EXPECT_EQ(c.value(), true);
    b = 6;
    // a = 6;  //a is readonly
}
