#include <mpi.h>
#include <random>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include "swarm.h"
#include "cost_function.h"
#include "constants.h"

swarm::swarm(const int input_num_particles, const int input_num_dim, int input_max_iter, 
   cost_func_base *input_ptr, const bounds &input_bnds, const int input_num_proc, const int in_proc_id)
   : max_iter{input_max_iter}, num_particles{input_num_particles}, p_cost_func{input_ptr}, 
   num_dim{input_num_dim}, num_proc{input_num_proc}, proc_id{in_proc_id}, bnds{input_bnds}
{
   particle_instance.resize(input_num_particles, particle(input_num_dim));
   best_soln.resize(input_num_dim,0.0);
}

void swarm::init_swarm()
{
   std::minstd_rand0 generator(proc_id);
   std::vector<std::uniform_real_distribution<double> > distribution_pos;
   std::uniform_real_distribution<double> distribution_vel(0.0,bnds.upper_bnds[0]*0.001);  
   std::vector<double> rand_pos(best_soln.size(),0.0);
   std::vector<double> rand_vel(best_soln.size(),0.0);

   if(verbose)
   {
      std::cout << "Initializing Swarm for Process: " << proc_id << std::endl;
   }

   distribution_pos.reserve(num_dim);
   for(int i = 0; i < num_dim; ++i)
   {
      distribution_pos.emplace_back(bnds.lower_bnds[i],bnds.upper_bnds[i]);
   }

   for(auto it = particle_instance.begin(); it != particle_instance.end(); it++)
   {
      int i = 0;
      std::generate(rand_pos.begin(), rand_pos.end(), [&]() {
         ++i;
         return distribution_pos[i-1](generator);
      });

      it->set_position(rand_pos);
      it->set_best_position(rand_pos);

      //check if particle's current position gives a smaller objective than 
      //the swarm's current best position
      if(cost_function(rand_pos, p_cost_func) < cost_function(best_soln, p_cost_func)) 
      {
         set_best_soln(rand_pos);
      }

      // std::cout << proc_id << ": " << std::setprecision(10) << best_soln[0] << std::endl;

      std::generate(rand_vel.begin(), rand_vel.end(), [&]() {
         return distribution_vel(generator);
      });          
      it->set_velocity(rand_vel);
   }

   find_and_exchange_global_best_soln();

   if(verbose)
   {
      std::cout << "Completed Swarm Initialization for process: " << proc_id << std::endl;
   }   
}

void swarm::set_best_soln(const std::vector<double> &input_best)
{
   best_soln = input_best;
}

void swarm::update_swarm()
{
   for(auto it = particle_instance.begin(); it != particle_instance.end(); it++)
   {
      it->update_velocity(best_soln, proc_id);
      it->update_position();

      if(cost_function(it->get_position(), p_cost_func) < cost_function(it->get_best_position(), p_cost_func))
      {
         it->set_best_position(it->get_position());

         //this part is the part that needs to be parallelized
         if(cost_function(it->get_position(), p_cost_func) < cost_function(best_soln, p_cost_func))
         {
            best_soln = it->get_position();
         }
      }
   }

   find_and_exchange_global_best_soln();
}

void swarm::iterate()
{
   int iter = 0;

   //iterate until max_iter is reached or until some other convergence criteria is reached;
   //latter is not yet implemented
   while(iter < max_iter)
   {
      if(verbose && proc_id == 0)
      {
         for(auto it = best_soln.begin(); it != best_soln.end(); it++)
         {
            std::cout << "Iteration number: " << iter << " | " << "Best Solution: " 
                      << std::setprecision(10) << *it << std::endl;
         }
      }

      update_swarm();

      iter++;
   }
}

void swarm::send_local_proc_best_soln()
{
   if(proc_id == 0)
   {
      std::cout << "ERROR: process " << proc_id << " calling send_local_proc_best_soln " << std::endl;
      exit(0);
   }

   // std::cout << best_soln[0] << std::endl;
   MPI_Send(best_soln.data(), num_dim, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD); //sends to process 0
}

void swarm::receive_local_proc_best_soln(const int source_proc_id, 
   std::vector<double> &recv_soln, MPI_Request &request)
{
   MPI_Irecv(recv_soln.data(), num_dim, MPI_DOUBLE, source_proc_id, 0, MPI_COMM_WORLD, &request); 
}

void swarm::send_global_best_soln(const int dest_proc_id, MPI_Request &request)
{
   if(proc_id > 0)
   {
      std::cout << "ERROR: process " << proc_id << " calling send_global_best_soln " << std::endl;
      exit(0);
   }

   MPI_Isend(best_soln.data(), num_dim, MPI_DOUBLE, dest_proc_id, 0, MPI_COMM_WORLD, &request);   
}

void swarm::receive_global_best_soln()
{
   //processes with proc_id > 0 receives best solution from process 0
   MPI_Recv(best_soln.data(), num_dim, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
}

void swarm::find_and_exchange_global_best_soln()
{
   //Send all proc_id > 0 processes best solution to proc_id = 0
   if(proc_id != 0 && num_proc > 1)
   {
      send_local_proc_best_soln();
   }

   int num_requests = num_proc - 1;
   if(proc_id == 0 && num_proc > 1)
   {
      //define a vector of receive requests for process 0
      std::vector<MPI_Request> rcv_requests(num_requests);

      //define a vector of received solutions from all proc_id > 0
      std::vector<std::vector<double> > recv_solns(num_proc - 1, std::vector<double>(num_dim, 0.0));

      for(int source_proc_id = 1; source_proc_id < num_proc; ++source_proc_id)
      {
         receive_local_proc_best_soln(source_proc_id, recv_solns[source_proc_id - 1], rcv_requests[source_proc_id - 1]);
      }

      //wait for process 0 to receive ALL local process best solns
      MPI_Waitall(num_requests, rcv_requests.data(), MPI_STATUSES_IGNORE);

      //now find which solution amongst the received, as well as this process's best solution, is
      //the best
      for(auto it = recv_solns.begin(); it != recv_solns.end(); ++it)
      {
         auto local_best_soln = *it;
         // std::cout << "proc 1: " << local_best_soln[0] << std::endl;
         // std::cout << "proc 0: " << best_soln[0] << std::endl;

         if(cost_function(local_best_soln, p_cost_func) < cost_function(best_soln, p_cost_func))
         {
            best_soln = local_best_soln;
         }         
      }

      //after finding the global best solution, we must now send this to all proc_id > 0
      std::vector<MPI_Request> send_requests(num_requests);
      for(int dest_proc_id = 1; dest_proc_id < num_proc; ++dest_proc_id)
      {
         send_global_best_soln(dest_proc_id, send_requests[dest_proc_id - 1]);
      }

      MPI_Waitall(num_requests, send_requests.data(), MPI_STATUSES_IGNORE);
   }

   //after sending the global best solution to all proc_id > 0, we must now receive this solution
   //for all proc_id > 0
   if(proc_id != 0 && num_proc > 1)
   {
      receive_global_best_soln();
   }
}
