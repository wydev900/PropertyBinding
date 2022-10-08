#pragma once
#include "Property.hpp"

struct Item {
    property<int> x;
    property<int> y;
    property<int> width;
    property<int> height;
};

struct Rectangle  {
    property<int> x;
    property<int> y;
    property<int> width;
    property<int> height;
    property<int> color;
};