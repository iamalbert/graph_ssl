#ifndef SSL_H
#define SSL_H

#include <cstdio>
#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
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
    inline bool operator<( const LabelProb & rhs ) const {
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

struct Distribution {
    std::vector< real > prob;
    inline size_t size() const {
        return prob.size();
    }
    inline real getProb(int i) const {
        return prob[i];
    }
};

struct EstimatedDistribution {
    size_t n, k;
    std::priority_queue< LabelProb > heap;
    real sum;
    std::unordered_set<int> indices;

    EstimatedDistribution( int n, int k ) ;

    inline real getEstimatedProb(int=0) const {
        return ( 1 - sum ) / ( n - k );
    }
    inline real getExactProb( int i ) const {
        const auto & c = get_container(heap);
        return std::find_if( RANGE(c), [&]( const LabelProb & o) {
            return o.index == i;
        })->value;
    }

    inline bool isEstimated(int i) const {
        return indices.find(i) == indices.end();
    }

    inline real getProb( int i ) const {
        if( isEstimated(i) ){
            return getEstimatedProb();
        }else{
            return getExactProb(i);
        }
    }

    EstimatedDistribution & setProb( int i , real p );
    real distDiff( const EstimatedDistribution & rhs ) const ;
    real distDiff( const Distribution & other ) const ; 
};

struct Node : public EstimatedDistribution {
};

using WeighedEdge = IndexedValue<int, real>;

struct Graph {
    std::vector< Node > nodes;
    std::vector< Node > nodes_buffer;

    std::vector< std::vector<WeighedEdge> > neighbors;

    std::vector< std::pair<int, Node> > seeds;
    std::unordered_map<int, int> seed_indices;

    Distribution prior;

    real coef[3];

    real computeObjective() const ;
    void prepare();
    inline bool is_seed(int i) const {
        return seed_indices.find(i) != seed_indices.end();
    }
    void update();
};
#endif
