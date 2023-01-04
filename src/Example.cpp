#include <cassert>
#include <cstdio>
#include "functions.h"

int main() {
    printf("Good example:\n");

    try {
        auto result = Functions::division(1,0);
        printf("result = %f\n", result);
    } catch (Swift::Error& e) {
        auto errorOpt = e.as<Functions::DivByZero>();
        assert(errorOpt.isSome());

        auto errorVal = errorOpt.get();
        assert(errorVal == Functions::DivByZero::divisorIsZero);
        errorVal.getMessage();
    }

    try {
        auto result = Functions::division(0,0);
        printf("result = %f\n", result);
    } catch (Swift::Error& e) {
        auto errorOpt = e.as<Functions::DivByZero>();
        assert(errorOpt.isSome());

        auto errorVal = errorOpt.get();
        assert(errorVal == Functions::DivByZero::bothAreZero);
        errorVal.getMessage();
    }

    try {
        float result = Functions::division(4,2);
        printf("result = %f\n", result);
    } catch (Swift::Error& e) {
        auto errorOpt = e.as<Functions::DivByZero>();
        assert(errorOpt.isSome());

        auto errorVal = errorOpt.get();
        errorVal.getMessage();
    }
    return 0;
}

