EXL ‒ Exceptionless template library
====================================
[![Build Status](https://travis-ci.org/pacmancoder/exl.svg?branch=master)](https://travis-ci.org/pacmancoder/exl)
[![Build status](https://ci.appveyor.com/api/projects/status/6072tjhi3js0tp9a/branch/master?svg=true)](https://ci.appveyor.com/project/pacmancoder/exl/branch/master)
[![Coverage](https://codecov.io/gh/pacmancoder/exl/branch/master/graph/badge.svg)](https://codecov.io/gh/pacmancoder/exl)

Exl library provides rich data manipulation abstractions without involving exceptions for its work.
#### Main Features
- Fast - Built with runtime efficiency in mind.
- Strict - Will detect and force to avoid many errors before runtime.
- Compact - No third-party dependencies. Only STL is required.
- Modern - Written mostly in C++11 standard.
- Lightweight - No implicit heap allocations, small footprint.
- Reliable - Full test coverage

### exl::mixed - std::variant on steroids
```cpp
// Error hierarchy
class Error { /* ... */ };
class ErrorA : public Error { /* ... */ };
class ErrorB : public Error { /* ... */ };

// ===============================================================

// Handy typedef of complex exl::mixed for this example.
// It can handle any of these types and occupies only memory
// which equals size of largest type + 1 byte for type tag
using Mixed = 
  exl::mixed<int, char ErrorA, ErrorB, Error, std::string>;

// Even more complex type. 
// Mixed can be assigned to MixedSuperset even type list has 
// different order!
using MixedSuperset = 
    exl::mixed<ErrorA, ErrorB, Error, std::string, int, std::string, char>;

// Function which performs some work and returns mixed type
Mixed do_calculatiions();

// ===============================================================

// Let's do value mapping! It's similar to catching exceptions. But
// without using exceptions at all. And "catched" type could be anything.

// Note that if we will not cover all possible types with matchers,
// code will not compile at all to avoid possible errors on runtime.

auto value = do_calculations.map<std::string>(
    exl::when_exact<int>([](int number)
    {
        // catch only int value
        return std::string("int: ").append(std::to_string(number));
    }),
    exl::when_exact<char>([](int ch)
    {
        // catch only char value
        return std::string("char: ").append(std::to_string(ch));
    }),
    exl::when_exact<ErrorB>(const ErrorB&)
    {
        // Only ErrorB will be catched
        return std::string("Catched ErrorB");
    }),
    exl::when<Error>(const Error&)
    {
        // Catching same or derived type!
        // either ErrorA or Error will bi catched
        return std::string("Catched some error...");
    }),
    exl::otherwise([]()
    {
        // This branch will be called if suitable 
        // matcher weren't found above
      	return std::string("Catched something at least...");
    })
);

// ===============================================================

// Initialize MixedSuperset with Mixed type - Yep. Also possible.
MixedSuperset superset(do_calculations());
```
