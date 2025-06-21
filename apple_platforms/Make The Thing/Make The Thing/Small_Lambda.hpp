//
//  Small_Lambda.hpp
//  Make The Thing
//
//  Created by Karl Rosenberg on 6/18/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef Small_Lambda_h
#define Small_Lambda_h

#include <cstddef>
#include <utility>
#include <new>

namespace my {

template <class Func, size_t Size = 128>
class Function;

template <class ReturnType, class...ArgTypes, size_t Size>
class Function<ReturnType (ArgTypes...), Size>
{
private:
    struct Interface
    {
        virtual ReturnType operator () (ArgTypes...) const = 0;
    };

    template <class Func>
    struct Implementation final : Interface
    {
        Func f;
        Implementation(const Func& f) : f(f) {}
        virtual ReturnType operator () (ArgTypes...args) const override
        {
            return f(args...);
        }
        
        virtual ~Implementation() = default;
    };
        
    alignas(std::max_align_t) char Storage[Size];

public:
    template <class Func>
    Function(const Func& f)
    {
        static_assert(sizeof(Implementation<Func>) <= Size);
        new (Storage) Implementation<Func>(f);
    }
    
    template <class Func>
    Function(const Function& other) = delete;
    
    template <class Func>
    Function(Function&& other) = delete;
//    
//    template <class Func>
//    Function& operator=(const Function& other) = delete;
//    
//    template <class Func>
//    Function& operator=(const Function& other) = delete;
    
    template <class Func>
    Function& operator=(Function &&other) = delete;
    
    
//    template <class Func>
//    Function(const Function& other)
//    {
//        new (Storage) Implementation<Func>(reinterpret_cast<Implementation<Func>&>(other.Storage));
//    }
//
//    template <class Func>
//    Function(Function&& other)
//    {
//        new (Storage) Implementation<Func>(std::move(reinterpret_cast<Implementation<Func>&>(other.Storage)));
//    }
//
//    template <class Func>
//    Function& operator=(const Function& other)
//    {
//        if (this == &other) return *this;
//        reinterpret_cast<Interface&>(Storage).~Interface();
//        new (Storage) Implementation<Func>(reinterpret_cast<Implementation<Func>&>(other.Storage));
//        return *this;
//    }
//
//    template <class Func>
//    Function& operator=(Function &&other)
//    {
//        if (this == &other) return *this;
//        reinterpret_cast<Interface&>(Storage).~Interface();
//        new (Storage) Implementation<Func>(std::move(reinterpret_cast<Implementation<Func>&>(other.Storage)));
//        return *this;
//    }

    template <class...CallArgTypes>
    ReturnType operator () (CallArgTypes&&...args) const
    {
        return reinterpret_cast<const Interface&>(Storage)(std::forward<CallArgTypes>(args)...);
    }
};

//#include <stdio.h>
//#include <vector>
//
//static int test(void)
//{
//    std::vector<Function<void(int)>> functions;
//    
//    // lambda without any captures
//    auto noncapture = [](int x) { printf("noncapture: x=%d\n", x); };
//    functions.emplace_back(noncapture);
//
//    // capture only by value
//    int y = 10;
//    auto captureval = [=](int x) { printf("captureval: x=%d y=%d\n", x, y); };
//    functions.emplace_back(captureval);
//
//    // capture by reference
//    auto captureref = [&](int x) { printf("captureref: x=%d y=%d\n", x, y++); };
//    functions.emplace_back(captureref);
//
//    // overwrite previous lambda with bigger lambda
//    int a=1, b=2, c=3, d=4, e=5, f=6;
//    functions.back() = [&](int x) { printf("captureref2: x=%d y=%d\n", x, y+=a+b+c+d+e+f); };
//
//    // Function<..> template can be copied around without problems
//    std::vector<Function<void(int)>> copy = functions;
//    functions.swap(copy);
//    copy.clear();
//
//    // do the actual calls
//    int index = 0;
//    for (const auto& f : functions)
//    {
//        f(index++);
//    }
//
//    // y should be 10 + (1+2+3+4+5+6) = 10 + 21 = 31
//    printf("end y=%d\n", y);
//}
}

#endif /* Small_Lambda_h */
