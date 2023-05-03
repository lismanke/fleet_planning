#ifndef FLEET_GRAPH_HH
#define FLEET_GRAPH_HH

#include "fleet_graph_prop.hh"
#include "../data/data.hh"

#include <stdio.h>
#include <stdlib.h>
#include <map>

#include <vector>
#include <unordered_map>
#include <numeric>

using namespace std;
using namespace boost;

using index_type = std::vector<int>;

struct index_hash {
    std::size_t operator()(index_type const& i) const noexcept {
        auto const hash_combine = [](auto seed, auto x) {
            return std::hash<int>()(x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        };
        return std::accumulate(i.begin() + 1, i.end(), i[0], hash_combine);
    }
};

template <typename T>
using sparse_array = std::unordered_map<index_type, T, index_hash>;

//class to store the network flow graph
class FleetGraph
{
	private:

		Fleet_Planning_Graph graph;

		property_map<Fleet_Planning_Graph, vertex_age_t>::type vertex_age = get(vertex_age_t(), graph);
		property_map<Fleet_Planning_Graph, vertex_class_t>::type vertex_class = get(vertex_class_t(), graph);
		property_map<Fleet_Planning_Graph, vertex_flow_cons_t>::type vertex_flow_cons = get(vertex_flow_cons_t(), graph);
		property_map<Fleet_Planning_Graph, vertex_time_t>::type vertex_time = get(vertex_time_t(), graph);

		// edge type discription:
		// o - own
		// p - purchase
		// r - retrofit
		// s - sell
		property_map<Fleet_Planning_Graph, edge_type_t>::type edge_type = get(edge_type_t(), graph);
		property_map<Fleet_Planning_Graph, edge_cost_t>::type edge_cost = get(edge_cost_t(), graph);
    property_map<Fleet_Planning_Graph, edge_var_t>::type edge_var = get(edge_var_t(), graph);

		adjacency_list<>::vertex_descriptor p_market_node;
		adjacency_list<>::vertex_descriptor r_market_node;
		sparse_array<boost::adjacency_list<>::vertex_descriptor> node_list;

	public:
		//Creates a fleet planning graph
		FleetGraph( int T_max,
							  Market_Situation& market,
							  vector<Aircraft_Properties>& aircrafts,
							  vector<Net_Class>& net_classes);

		//Deconstructor

		~FleetGraph()
		{}

		Fleet_Planning_Graph& get_graph()
		{
			return(graph);
		}

		adjacency_list<>::vertex_descriptor get_p_market_node() const
		{
			return(p_market_node);
		}

		adjacency_list<>::vertex_descriptor get_r_market_node() const
		{
			return(r_market_node);
		}

		adjacency_list<>::vertex_descriptor get_node(int t, int c, int a)
		{
			return(node_list[index_type{t,c,a}]);
		}

		char get_edge_type(const Fleet_Planning_Graph::edge_descriptor& edge)
		{
			return(get(edge_type,edge));
		}

		double get_edge_cost(const Fleet_Planning_Graph::edge_descriptor& edge)
		{
			return(get(edge_cost,edge));
		}

		int get_vertex_age(const adjacency_list<>::vertex_descriptor& vertex)
		{
			return(get(vertex_age,vertex));
		}

		int get_vertex_class(const adjacency_list<>::vertex_descriptor& vertex)
		{
			return(get(vertex_class,vertex));
		}

		int get_vertex_time(const adjacency_list<>::vertex_descriptor& vertex)
		{
			return(get(vertex_time,vertex));
		}

		void var_to_edge(SCIP_VAR* var, const Fleet_Planning_Graph::edge_descriptor& edge)
		{
			boost::put(edge_var, edge, var);
		}

		void flow_cons_to_vertex(SCIP_CONS* cons, const adjacency_list<>::vertex_descriptor& vertex)
		{
			boost::get(vertex_flow_cons,vertex).push_back(cons);
		}

		SCIP_VAR* get_var_of_edge(const Fleet_Planning_Graph::edge_descriptor& edge)
		{
			return(boost::get(edge_var,edge));
		}

		SCIP_RETCODE release_var_at(SCIP* scip, const Fleet_Planning_Graph::edge_descriptor& edge)
		{
			SCIP_VAR* t_var = boost::get(edge_var,edge);
      SCIP_RETCODE released;
			if(&t_var != NULL)
      {
        released = SCIPreleaseVar(scip, &t_var);
      }
      return(released);
		}
};

#endif // FLOW_GRAPH_HH
