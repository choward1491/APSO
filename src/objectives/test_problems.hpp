//
//  test_problems.hpp
//  async_pso
//
//  Created by Christian Howard on 7/1/19.
//  Copyright © 2019 Christian Howard. All rights reserved.
//

#ifndef test_problems_hpp
#define test_problems_hpp

namespace test {
    static std::mt19937 gen;
    struct levyfunc_n13 { double operator()(const std::vector<double>& x) const; };
    struct quadratic { double operator()(const std::vector<double>& x) const; };
}

#endif /* test_problems_hpp */
