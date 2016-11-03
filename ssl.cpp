#include <cstdio>
#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <unordered_set>
#include <queue>
#include <list>

#define RANGE(c) std::begin(c), std::end(c)

typedef double real;

template<class Index, class Value>
struct IndexedValue {
    Index index;
    Value value;

    IndexedValue( const Index & i, const Value & v) : index(i), value(v) {
    }
};
struct LabelProb : public IndexedValue<int, real> {
    using IndexedValue<int, real>::IndexedValue;
    bool operator<( const LabelProb & rhs ) const {
        return value > rhs.value;
    }
};

template<class T>
auto get_container( const T & obj ) -> typename T::container_type {
    struct Hack : public T {
        static const typename T::container_type & gc( const T & obj ){
            return obj.*&Hack::c;
        };
    } ;
    return Hack::gc(obj);
}

struct LogNumber {
    real value;

    LogNumber & operator*=( const LogNumber & x ){ value += x.value; return *this;}
    LogNumber & operator/=( const LogNumber & x ){ value -= x.value; return *this;}

};

struct Distribution {
    std::vector< real > prob;
};

struct EstimatedDistribution {
    int n, k;
    std::priority_queue< LabelProb > heap;
    real sum;
    std::unordered_set<int> indices;

    EstimatedDistribution( int n, int k ) : n(n), k(k), sum(0) {
    }

    real getEstimatedProb(int=0) const {
        return ( 1 - sum ) / ( n - k );
    }
    real getExactProb( int i ) const {
        const auto & c = get_container(heap);
        return std::find_if( RANGE(c), [&]( const LabelProb & o) {
            return o.index == i;
        })->value;
    }

    bool isEstimated(int i) const {
        return indices.find(i) == indices.end();
    }

    real getProb( int i ) const {
        if( isEstimated(i) ){
            return getEstimatedProb();
        }else{
            return getExactProb(i);
        }
    }

    EstimatedDistribution & setProb( int i , real p ) {
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

    real distDiff( const EstimatedDistribution & rhs ) const {
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

};

struct Node : public EstimatedDistribution {
    Node() : EstimatedDistribution(10, 5) {}
};

using WeighedEdge = IndexedValue<int, real>;

struct Graph {
    std::vector< Node > nodes;
    std::vector< std::vector<WeighedEdge> > neighbors;
    std::unordered_set< int > seed_indices;

    Distribution prior;

    real coef[3];


    real computeObjective(){
        real seedDist = 0;
        real neighborDist = 0;
        real labelDist = 0;

        int i = 0;
        #pragma omp parallel for private(i)
        for( i=0; i < nodes.size(); i++ ){

            auto & node = nodes[i];
            auto & neighors = neighbors[i];
            bool is_seed = ( seed_indices.find(i) == seed_indices.end() );

            real sd = 0, nd = 0, ld = 0;

            for( int j = 0; j < i*100; j++ ){
                sd += 1;
            }

            #pragma omp critical 
            {
                seedDist += sd;
                neighborDist += nd;
                labelDist += ld;
            }
        }

        return coef[0]*seedDist + coef[1]*neighborDist + coef[2]*labelDist;
    }
};

int main(){
    Graph graph;
    graph.nodes.resize(100000);
    graph.computeObjective();
}
