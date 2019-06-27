//
//  particle.hpp
//  async_pso
//
//  Created by Christian Howard on 6/26/19.
//  Copyright © 2019 Christian Howard. All rights reserved.
//

#ifndef particle_hpp
#define particle_hpp

#include <random>
#include <vector>

namespace async {
    namespace pso {
        
        class particle {
        public:
            
            // ctor/dtor
            particle(int dim = 0);
            ~particle() = default;
            
            // set number of dims for particle
            void set_num_dims(int dim);
            void set_momentum(double omega);
            void set_particle_weights(double phi_local, double phi_global);
            
            // initialize
            void initialize(std::default_random_engine& gen,
                            const std::vector<double>& lb,
                            const std::vector<double>& ub);
            
            // update the particle state
            void update(const std::vector<double>& global_best);
            
            // set the function value for the particle
            void set_function_value(double fval);
            
            // get the current function value or state
            double get_current_val() const;
            const std::vector<double>& get_current_position();
            
            // get the current best states for this particle
            double get_best_val() const;
            const std::vector<double>& get_best_position() const;
            
        private:
            double func_val, w, phi_l, phi_g;
            std::vector<double> p;
            std::vector<double> v;
            const std::vector<double> *lb;
            const std::vector<double> *ub;
            std::default_random_engine* gen;
            
            double best_val;
            std::vector<double> best_p;
            
        };
        
    }// end namespace pso
}// end namespace async

#endif /* particle_hpp */
