#ifndef _cost_func_h_
#define _cost_func_h_

#include <math.h>
#include <vector>

//Implementation of a functor class; base class is abstract -> can't be instantiated
//

class cost_func_base
{
   public:
      cost_func_base() = default;
      virtual double operator() (std::vector<double> &vec) const = 0; //pure virtual
};   

class x_sq : public cost_func_base
{
   static inline double calc_sq(double x) {return (x-0.5)*(x-0.5) ;}

   public:
      x_sq() = default;
      virtual double operator()(std::vector<double> &vec) const
      {
          double output = 0.0;
          for(auto v: vec){
              output += calc_sq(v);
          }
         return output;
      }
    
};         

inline double cost_function(std::vector<double> &vec, const cost_func_base * const user_cost)
{
   return (*user_cost)(vec);
}

#endif //_cost_func_h_
