# Swift and C++ Interop: Handling Error on Division Function
The Swift Compiler now has the feature of handle `Swift Errors` as `C++ Exceptions` or to return a Class that represents this error. Using either of these two approaches, C++ users can handle errors thrown by Swift functions. Below, the straightforward example shows a Swift function that can throw an error to avoid unexpected behavior.

```Swift
/// Example of a user-defined Error and a Swift function that can throw it.

@_expose(Cxx)
public enum DivByZero : Error {
    case divisorIsZero
    case bothAreZero

    // Function to print the case thrown
    public func getMessage() {
        print(self)
    }
}

@_expose(Cxx)
public func division(_ a: Int, _ b: Int) throws -> Float {
    if a == 0 && b == 0 {
        throw DivByZero.bothAreZero
    } else if b == 0 {
        throw DivByZero.divisorIsZero
    } else {
        return Float(a / b)
    }
}
```

The client of this function, the C++ user, must first compile the program in which this function is defined to create the bridging header to export the Swift implementation to C++. The below command can be used to generate this header; it was extracted from the Swift test suit and simplified as much as possible.

```bash
# Command to create the bridging header of a Swift program.

$ swift-frontend Example.swift -typecheck -module-name Functions \
    -enable-experimental-cxx-interop -emit-clang-header-path functions.h
```

This command takes the "Example.swift" file as input and outputs the "functions.h" file with the C++ representation of the Swift functions under the "Functions" namespace.

```C++
/// Example of using a Swift function that can throw an Error in C++ that can be caught.

inline Swift::ThrowingResult<float> division(swift::Int a, swift::Int b) {
  void* opaqueError = nullptr;
  void* _ctx = nullptr;
  auto returnValue = _impl::$s9Functions8divisionySfSi_SitKF(a,b,_ctx,&opaqueError);
  if (opaqueError != nullptr)
#ifdef __cpp_exceptions
    throw (Swift::Error(opaqueError));
#else
    return SWIFT_RETURN_THUNK(float, Swift::Error(opaqueError));
#endif

  return SWIFT_RETURN_THUNK(float, returnValue);
}
```

 The C++ function represented  generated by the compiler, and shown above, can be explained in four parts:

 - **Function signature**: The division function has the type "_**ThrowingResult<float>**_" instead of the "_**float**_" type declared on Swift. The implementation of this type is shown on listing \ref{lst:ThrowingResult}, and it is used to avoid code repeating by declaring the function twice with different types to satisfy the two error handling approaches.

 - **The Swift function call**: This has two extra arguments we defined before; the first holds the context, and the second the error information. They are required to match the Swift ABI, and the last argument has the core information to specify if the Swift function threw an error and which error it is or if it returned as expected.

 - **The opaqueError is nullptr?**: This if condition checks if the "_**opaqueError**_" argument was modified during the execution of the function to hold Swift error information or not. Also, inside the branch, we have two different implementations regarding the use of exceptions by the C++ user; if it's enabled, we throw a "_**Swift::Error**_" object initiated with the "_**opaqueError**_" argument. If not, we **return** a macro that will be replaced by "_**Swift::Expected<float>(Swift::Error(opaqueError))**_".

 - **The return expression**: The macro returned here will be replaced at compile time depending on the use of C++ exceptions. It can either only returns the "_**returnValue**_" or the "_**Swift::Expected<float>(returnValue)**_" that contains the value we expected instead of an Error as it previous use in this example.


The C++ programmer uses this function representation by importing the generated Swift bridging header to its program. Below we show two uses of this function using C++ exceptions:

```C++
/// Examples of using a Swift function that can throw an error in C++ and handles it.

#include <cassert>
#include <cstdio>
#include "functions.h"

int main() {

    // This example catches an exception
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

    // This example gets the correct value returned by the function
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
```

In this example, we can see how to catch the "_**Swift::Error**_", dynamically cast it to the user-defined error, which returns an Optional type that says if the casting was successful or not, check if it threw the case that we expected, and then, tells the user name of it using the "_**getMessage()**_" function. If the function doesn't throw an error, we get the "_**float**_" result and print it to the standard output.

The commands below show how to compile and test this program. First, we need to compile and generate its object file, compile it with the Swift file containing the function and error definitions, and then generate the executable program.

```Bash
# Commands to build, link and execute a C++ program that calls a Swift function and can throw exceptions.

$ clang++ -I ${SWIFT_BUILD_DIR}/swift-macosx-arm64/./lib/swift \
        -c Example.cpp -I /. -o example.o

$ swiftc -Xfrontend -enable-experimental-cxx-interop Example.swift -o example \
        -Xlinker example.o -module-name Functions -Xfrontend \
        -entry-point-function-name -Xfrontend swiftMain

$ ./example

```

The second approach to handle Swift errors in C++ was inspired by the C++ proposal "[p0323r4 std::expected](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0323r4.html#unexpected.synop)". This approach required a new C++ parameterized class called "Swift::Expected<T>", which can be initialized to hold either a Swift::Error or a value of type T, never simultaneously. This class has auxiliary functions to improve the interface with the user and simplify the test and get an error or a value. An instance of this class is returned by a C++ representation of a Swift function that can throw an error, but the C++ user disabled exceptions at compile time.

The example below uses the same Swift function and C++ representation of it but with this new error-handling model:

```C++
/// Handling Swift Error using an expected type based on the std::expected proposal

#include <cassert>
#include <cstdio>
#include "functions.h"

int main() {
    auto errorResult = Functions::division(1,0);
    if (errorResult.has_value()) {
        printf("result = %f\n", errorResult.value());
    } else {
        auto optionalError = errorResult.error().as<Functions::DivByZero>();
        assert(optionalError.isSome());

        auto errorValue = optionalError.get();
        assert(errorValue == Functions::DivByZero::divisorIsZero);
        errorValue.getMessage();
    }

    auto goodResult = Functions::division(4,2);
    if (goodResult.has_value()) {
        printf("result = %f\n", goodResult.value());
    } else {
        auto optionalError = goodResult.error().as<Functions::DivByZero>();
        assert(optionalError.isSome());

        auto errorValue = optionalError.get();
        errorValue.getMessage();
    }
    return 0;
}
```

The main difference in this example is that the "try-catch" block is not used. Instead, the function's return is tested to check if it contains a value, the good case where we just print the value or not. This last case is most interesting, as the error was returned and can be accessed by the "error()" function and then handled as before.

The commands below show how to compile and test this program. First, we need to compile and generate its object file with the "_-fno-exceptions_" flag, compile it with the Swift file containing the function and error definitions and then generate the executable program.

```Bash
# Commands to build, link, and execute a C++ program that calls a Swift function and can't throw exceptions, instead, it uses the Swift::Expected<T> class.

$ clang++ -I ${SWIFT_BUILD_DIR}/swift-macosx-arm64/./lib/swift \ 
        -c Expected_Example.cpp -fno-exceptions -I /. -o expected_example.o

$ swiftc -Xfrontend -enable-experimental-cxx-interop Example.swift \
        -o expected_example -Xlinker expected_example.o -module-name Functions \
        -Xfrontend -entry-point-function-name -Xfrontend swiftMain

$ ./expected_example
```
