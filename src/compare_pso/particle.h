#ifndef _particle_pso_h_
#define _particle_pso_h_

#include <vector>

class particle
{
   public:
      particle() = default;
      particle(const int input_num_dim)
      {
         // num_dim = input_num_dim;
         velocity.resize(input_num_dim,0.0);
         position.resize(input_num_dim,0.0);
         best_position.resize(input_num_dim,0.0);
      }

      void set_velocity(const std::vector<double> &input_velocity);
      void set_position(const std::vector<double> &input_position);
      void set_best_position(const std::vector<double> &input_position);

      //updates position using x(t+1) = x(t) + v(t+1)
      void update_position();

      //update velocity using v(t+1) = ....
      void update_velocity(const std::vector<double> &swarm_best, const int proc_id);
      
      std::vector<double> get_position();
      std::vector<double> get_best_position();


   private:
      // int num_dim;
      std::vector<double> velocity;
      std::vector<double> position;
      std::vector<double> best_position;
};

#endif //_particle_pso_h_