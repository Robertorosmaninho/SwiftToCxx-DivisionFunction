#include <cassert>
#include <cstdio>
#include "functions.h"

int main() {
    printf("Runnnig expected example:\n");

    auto result0 = Functions::division(1,0);
    if (result0.has_value()) {
        printf("result = %f\n", result0.value());
    } else {
        auto optionalError = result0.error().as<Functions::DivByZero>();
        assert(optionalError.isSome());

        auto errorValue = optionalError.get();
        assert(errorValue == Functions::DivByZero::divisorIsZero);
        errorValue.getMessage();
    }

    auto result1 = Functions::division(0,0);
    if (result1.has_value()) {
        printf("result = %f\n", result1.value());
    } else {
        auto optionalError = result1.error().as<Functions::DivByZero>();
        assert(optionalError.isSome());

        auto errorValue = optionalError.get();
        assert(errorValue == Functions::DivByZero::bothAreZero);
        errorValue.getMessage();
    }

    auto result2 = Functions::division(4,2);
    if (result2.has_value()) {
        printf("result = %f\n", result2.value());
    } else {
        auto optionalError = result2.error().as<Functions::DivByZero>();
        assert(optionalError.isSome());

        auto errorValue = optionalError.get();
        errorValue.getMessage();
    }
    return 0;
}

