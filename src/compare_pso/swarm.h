#ifndef _swarm_pso_h_
#define _swarm_pso_h_

#include <vector>
#include "particle.h"
#include "cost_function.h"

struct bounds
{
   std::vector<double> lower_bnds;
   std::vector<double> upper_bnds;

   bounds() = default;

   bounds(int num_dim, const std::vector<double> &input_lower_bnds, 
      const std::vector<double> &input_upper_bnds)
   {
      lower_bnds.resize(num_dim);
      upper_bnds.resize(num_dim);

      set_bounds(input_lower_bnds, input_upper_bnds);
   }

   void set_bounds(const std::vector<double> &input_lower_bnds, 
      const std::vector<double> &input_upper_bnds)
   {
      lower_bnds = input_lower_bnds;
      upper_bnds = input_upper_bnds;
   }
};

class swarm
{
   public:
      swarm() = default;

      swarm(const int input_num_particles, const int input_num_dim, int input_max_iter, 
         cost_func_base *input_ptr, const bounds &input_bnds, const int input_num_proc, const int in_proc_id);

      //Member function for initializing all swarm particle velocities and positions
      //based on uniform random distributions
      void init_swarm();  

      //Setter function for the best (most optimal) solution at the current iteration
      void set_best_soln(const std::vector<double> &input_best);  

      //Member function for updating the swarm particles' velocity and position
      //Called during swarm::iterate()
      void update_swarm();

      //Member function for performing the iterative scheme, i.e., updates position and velocity
      void iterate();

      //function for sending local process' (for proc_id > 0) best soln to process 0.
      //process zero acts as a pseudo-master here for determining the global best soln
      //note this function should not be executed for proc_id > 0   
      //note also that blocking send is used in this function; there aren't any advantages to
      //using non-blocking here
      void send_local_proc_best_soln();

      //function for receiving local process' (for proc_id > 0) best soln
      //note this function should only be executed for proc_id = 0, and it should be executed
      //(num_proc - 1) times because we are receiving from (num_proc - 1) processors
      //non-blocking receive is used here 
      void receive_local_proc_best_soln(const int source_proc_id, 
         std::vector<double> &recv_soln, MPI_Request &request);

      //function for sending the global best solution, stored in proc_id = 0, to all proc_id > 0
      //Note this function is only executed for proc_id = 0
      //Also note this function uses NON-blocking send
      void send_global_best_soln(const int proc_id, MPI_Request &request);
      
      //function for receiving the global best solution from proc_id = 0
      //Note this fucntion is only executed for proc_id > 0
      //Also not ethis function uses blocking-send because proc_id 
      void receive_global_best_soln();

      //Wraps the above 4 functions
      void find_and_exchange_global_best_soln();
;
      std::vector<double> best_soln;
    
   private:
      int num_particles;
      int max_iter;
      int num_dim;   //# of dimensions
      int num_proc; //# of procesors
      int proc_id;  //process id
      std::vector<particle> particle_instance;
      cost_func_base *p_cost_func;
      bounds bnds;
};

#endif //_swarm_pso_h_
