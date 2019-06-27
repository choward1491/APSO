//
//  particle.cpp
//  async_pso
//
//  Created by Christian Howard on 6/26/19.
//  Copyright © 2019 Christian Howard. All rights reserved.
//

#include "particle.hpp"
#include <cmath>

namespace async {
    namespace pso {
            
        // ctor/dtor
        particle::particle(int dim):w(0.9),phi_l(0.7), phi_g(0.5){
            set_num_dims(dim);
        }
        
        // set number of dims for particle
        void particle::set_num_dims(int dim){
            p.resize(dim);
            v.resize(dim);
            best_p.resize(dim);
        }
        
        void particle::set_momentum(double omega) {
            w = omega;
        }
        void particle::set_particle_weights(double phi_local, double phi_global){
            phi_l = phi_local;
            phi_g = phi_global;
        }
        
        // initialize
        void particle::initialize(std::mt19937& gen_,
                        const std::vector<double>& lb_,
                        const std::vector<double>& ub_)
        {
            // set the references needed
            std::uniform_real_distribution<double> U(0,1);
            gen = &gen_;
            lb  = &lb_;
            ub  = &ub_;
            
            // loop and initialize the positions and velocities
            for(size_t i = 0; i < p.size(); ++i){
                double s = U(gen_), t = U(gen_);
                double del = std::abs(lb_[i] - ub_[i]);
                p[i] = lb_[i]*s + ub_[i]*(1-s);
                best_p[i] = p[i];
                v[i] = -del*t + del*(1-t);
            }
            
        }
        
        // update the particle state
        void particle::update(const std::vector<double>& global_best)
        {
            std::uniform_real_distribution<double> U(0,1);
            for(size_t i = 0; i < v.size(); ++i){
                double rl = U(*gen), rg = U(*gen);
                v[i] =  w * v[i]
                        + phi_l * rl * (best_p[i] - p[i])
                        + phi_g * rg * (global_best[i] - p[i]);
                p[i] += v[i];
                
                // if you go out of bounds, project back
                // onto the boundary and set velocity to zero
                if( p[i] < (*lb)[i] ){p[i] = (*lb)[i]; v[i] = 0.0; }
                if( p[i] > (*ub)[i] ){p[i] = (*ub)[i]; v[i] = 0.0; }
            }
        }
        
        // set the function value for the particle
        void particle::set_function_value(double fval) {
            func_val = fval;
            
            // update personal best, if necessary
            if( func_val < best_val ){
                best_val = func_val;
                for(size_t i = 0; i < p.size(); ++i){
                    best_p[i] = p[i];
                }
            }
        }
        
        // get the current function value or state
        double particle::get_current_val() const {
            return func_val;
        }
        const std::vector<double>& particle::get_current_position() {
            return p;
        }
        
        // get the current best states for this particle
        double particle::get_best_val() const {
            return best_val;
        }
        const std::vector<double>& particle::get_best_position() const {
            return best_p;
        }
            
        /*
            double func_val;
            std::vector<double> p;
            std::vector<double> v;
            
            double best_val;
            std::vector<double> best_p;
        */
        
    }// end namespace pso
}// end namespace async
