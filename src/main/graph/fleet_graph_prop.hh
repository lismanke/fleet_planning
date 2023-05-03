#ifndef FLEET_GRAPH_PROP_HH
#define FLEET_GRAPH_PROP_HH

#include <boost/graph/adjacency_list.hpp>

#include <objscip/objscip.h>
#include <scip/scip.h>

#include <iostream>
#include <vector>

using namespace std;
using namespace boost;

struct vertex_age_t {
  typedef vertex_property_tag kind;
};

struct vertex_class_t {
  typedef vertex_property_tag kind;
};

struct vertex_flow_cons_t {
  typedef vertex_property_tag kind;
};

struct vertex_time_t {
  typedef vertex_property_tag kind;
};

struct edge_type_t {
  typedef edge_property_tag kind;
};

struct edge_cost_t {
  typedef edge_property_tag kind;
};

struct edge_var_t {
  typedef edge_property_tag kind;
};

typedef property<edge_var_t, SCIP_VAR*> EdgeVar;
typedef property<edge_cost_t, double, EdgeVar> EdgeCost;
typedef property<edge_type_t, char, EdgeCost> EdgeProperty;

typedef property<vertex_age_t, int> VertexAge;
typedef property<vertex_class_t, int, VertexAge> VertexClass;
typedef property<vertex_flow_cons_t, vector<SCIP_CONS*>, VertexClass> VertexFlowCons;
typedef property<vertex_time_t, int, VertexFlowCons> VertexProperty;


typedef adjacency_list<setS, vecS, bidirectionalS, VertexProperty, EdgeProperty> Fleet_Planning_Graph;

#endif // FLEET_GRAPH_PROP_HH
