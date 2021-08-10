#pragma once

struct CalcData {
    double a;
    double b;
};

struct Result {
  double res;
};

union unData{
    struct CalcData args;
    struct Result res;
};
