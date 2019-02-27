
<p align="center">
    <img src="assets/img/logo_480.png"/>
    <br><br>
    <a href="https://travis-ci.org/pacmancoder/exl"><img src="https://travis-ci.org/pacmancoder/exl.svg?branch=master"/></a>
    <a href="https://ci.appveyor.com/project/pacmancoder/exl/branch/master"><img src="https://ci.appveyor.com/api/projects/status/6072tjhi3js0tp9a/branch/master?svg=true"/></a>
    <a href="https://codecov.io/gh/pacmancoder/exl"><img src="https://codecov.io/gh/pacmancoder/exl/branch/master/graph/badge.svg"/></a>
    <a href="https://www.codefactor.io/repository/github/pacmancoder/exl"><img src="https://www.codefactor.io/repository/github/pacmancoder/exl/badge"/></a>
</p>

### EXL ‒ Exceptionless template library
This template library provides set of types which will enforce to handle errors
explicitly without involving any kind of exceptions at all. Constructed with focus
on early error detection - a lot of errors will be detected during compilation. Client
code is encouraged to be safe - there is a plenty of safe data manipulation tools
like data mapping and matchers which are move powerfull than direct data access methods.
If code triggers invalid action - programm will terminate. There is no undefined behavior.


### Main Features
- Fast - Built with runtime efficiency in mind.
- Strict - Will detect and enforce to avoid many errors even before runtime.
- Compact - No third-party dependencies. Only STL is required.
- Modern - Written mostly in C++11 standard.
- Lightweight - No implicit heap allocations, small footprint.
- Reliable - Full test coverage
- Straightforward - No undefined behavior. If it fails then it crashes.

### Components
- exl::mixed - std::variant on steroids
- exl::option - handle optional data like a boss
- exl::box - more verbose and flexible std::varinat substitution

### Showcase: exl::mixed
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
