#include "ssl.h"


EstimatedDistribution::EstimatedDistribution( int n, int k ) : n(n), k(k), sum(0) {
    }

EstimatedDistribution & EstimatedDistribution::setProb( int i , real p ) {
    if ( heap.size() < k ){
        heap.push( {i,p} );
        indices.insert(i);
        sum += p;
    } else {
        auto & minValue = heap.top();
        if ( minValue.value >= p ){
            /* nothing */
        } else {
            indices.erase(minValue.index); heap.pop();
            indices.insert(i);             heap.push( {i,p} );
            sum = sum + p - minValue.value;
        }

    }
    return *this;
}

real EstimatedDistribution::distDiff( const EstimatedDistribution & rhs ) const {
    real sum = 0;

    real est_x = getEstimatedProb(), est_y = rhs.getEstimatedProb();

    for ( auto && lab : get_container(heap) ){
        real d = 0;
        if ( rhs.isEstimated(lab.index) ){
            d = lab.value - est_y;
        } else {
            d = lab.value - rhs.getEstimatedProb(lab.index);
        }
        sum += d*d;
    }

    int cnt = heap.size();
    for( auto && lab : get_container(rhs.heap) ){
        if ( rhs.isEstimated(lab.index) ){
            real d = lab.value - est_x;
            sum += d*d;
            cnt += 1;
        }
    }

    sum += (est_x - est_y)*(est_x - est_y) * ( n - cnt );

    return sum;
}

real EstimatedDistribution::distDiff( const Distribution & other ) const{
    real sum = 0;

    for( size_t i=0; i < other.size(); i++){
        real d = other.getProb(i) - getProb(i);
        sum += d*d;
    }

    return sum;
}




real Graph::computeObjective() const{
    real seedDist = 0;
    real neighborDist = 0;
    real labelDist = 0;

    size_t i;
    #pragma omp parallel for private(i)
    for( i=0; i < seeds.size(); i++ ){
        int index = seeds[i].first;
        const Node & seed = seeds[i].second;
        const Node & pred = nodes[index];

        real d = seed.distDiff(pred);

        #pragma omp critical 
        {
            seedDist += d;
        }

    }

    #pragma omp parallel for private(i)
    for( i=0; i < nodes.size(); i++ ){

        auto & node = nodes[i];
        auto & neighbor = neighbors[i];

        real nd = 0, ld = 0;

        for( auto & kv : neighbor ){
            auto dest   = kv.index;
            auto weight = kv.value;

            nd += weight * node.distDiff( nodes[dest] );
        }
        ld = node.distDiff(prior);

        #pragma omp critical 
        {
            neighborDist += nd;
            labelDist += ld;
        }
    }

    return coef[0]*seedDist + coef[1]*neighborDist + coef[2]*labelDist;
}

void Graph::prepare(){
    for( size_t i=0; i < seeds.size(); i++ ){
        seed_indices[ seeds[i].first ] =  i;
    }
}
void Graph::update(){

    std::copy( std::begin(nodes), std::end(nodes), std::begin(nodes_buffer) );

    size_t i;
    #pragma omp parallel for private(i)
    for( i=0; i < nodes.size(); i++ ){
        real m = 0;

        for( WeighedEdge & neighbor : neighbors[i] ){ 
            m += neighbor.value; 
        }
        m *= coef[1];

        m += coef[2];

        bool is_seed = this->is_seed(i);

        if( is_seed ) m += coef[0];

        Node & new_node = nodes[i];

        for( size_t l = 0; l < prior.size(); l++){

            real new_prob = 0;
            for( WeighedEdge & neighbor : neighbors[i] ){ 
                new_prob += nodes_buffer[ neighbor.index ].getProb(l) * neighbor.value; 
            }
            new_prob *= coef[1];
            
            new_prob += is_seed ? coef[0] * seeds[ seed_indices[i] ].second.getProb(l) : 0;
            new_prob += coef[2] * prior.getProb(l);

            new_prob /= m;

            new_node.setProb(l, new_prob);
        }
    }
}
