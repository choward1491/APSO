#include "particle.h"
#include <vector>
#include <functional> 
#include <random> 

void particle::set_velocity(const std::vector<double> &input_velocity)
{
   velocity = input_velocity;
}

void particle::set_position(const std::vector<double> &input_position)
{
   position = input_position;
}

void particle::set_best_position(const std::vector<double> &input_position)
{
   best_position = input_position;
}

void particle::update_position()
{
   std::transform(position.begin(), position.end(), velocity.begin(), position.begin(),
      std::plus<double>());
}

void particle::update_velocity(const std::vector<double> &swarm_best, const int proc_id)
{
   double c1 = 2; //make c1&x2 user-defined parametrs later
   double c2 = 3;
   static std::minstd_rand0 generator(proc_id); 
   static std::uniform_real_distribution<double> distribution(0.0,1.0);

   size_t num_dim = position.size();
   for(int i = 0; i < num_dim; ++i)
   {
      velocity[i] += c1*distribution(generator)*(best_position[i] - position[i])            
         + c2*distribution(generator)*(swarm_best[i] - position[i]);
   }
}

std::vector<double> &particle::get_best_position()
{
   return best_position;
}

std::vector<double> &particle::get_position()
{
   return position;
}
