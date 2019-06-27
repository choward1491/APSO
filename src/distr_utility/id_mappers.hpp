//
//  id_mappers.hpp
//  distributed_hybrid_meshing
//
//  Created by Christian Howard on 6/25/19.
//  Copyright © 2019. All rights reserved.
//

#ifndef id_mappers_hpp
#define id_mappers_hpp

namespace distributed {
    
    // round robin ID map
    struct roundrobin_idmap {
        int get_rank(size_t gID) {
            return gID % trank;
        }
        size_t get_local_id(size_t gID){
            return gID / trank;
        }
        size_t get_global_id(size_t local_id) {
            return lrank + local_id * trank;
        }
        void set_local_rank(int local_rank) { lrank = local_rank; }
        void set_total_ranks(int tot_rank) { trank = tot_rank; }
        int lrank, trank;
    };
    
}


#endif /* id_mappers_h */
