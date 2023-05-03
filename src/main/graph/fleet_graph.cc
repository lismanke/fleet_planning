#include "fleet_graph.hh"

#include <iostream>
#include <assert.h>

//Creates a fleet planning graph
FleetGraph::FleetGraph( int T_max,
												Market_Situation& market,
												vector<Aircraft_Properties>& aircrafts,
												vector<Net_Class>& net_classes)
{
  std::cout << "contruction of graph: \n";
	using i = index_type;
	//initialize market nodes
	p_market_node=boost::add_vertex(graph);
	r_market_node=boost::add_vertex(graph);

	//initialize nodes for aircrafts of type c at time t and age a
	for(int t=0;t<=T_max;t++)
	{
		for(int c=0;c<aircrafts.size();c++)
		{
			//skip retrofitted aircrafts at t=0 				& retrofitted types if retrofit is not available jet 																				& types which are not available jet
			if((aircrafts[c].is_retrofitted() && t<1) || (aircrafts[c].is_retrofitted() && t <= aircrafts[c].get_retro_purchasable_since_period()) || t <= aircrafts[c].get_purchasable_since_period())	continue;
			//create one node for each possible age of aircraft type (idea is to treat it like an own aircraft type)
			int service_t = aircrafts[c].get_service_t()+1;
			if(service_t > t-aircrafts[c].get_purchasable_since_period()) service_t = t-aircrafts[c].get_purchasable_since_period();
			for(int a=0;a<service_t;a++)
			{
				//skip types at service_t if available during optimization process
				if(aircrafts[c].get_purchasable_since_period() > 0 && a == service_t) continue;
				//skip retrofitted aircrafts at age=0
				if(aircrafts[c].is_retrofitted() && a==0)	continue;
				boost::adjacency_list<>::vertex_descriptor temp_node = boost::add_vertex(graph);
				node_list[i{t,c,a}] = temp_node;
				boost::put(vertex_age,temp_node,a);
				boost::put(vertex_class,temp_node,c);
				boost::put(vertex_time,temp_node,t);
			}
		}
	}

	//initialize edges for purchasing(p), selling(s), retrofitting(r) or doing nothing(o) with the aricraft types
	std::pair<Fleet_Planning_Graph::edge_descriptor, bool> temp_edge;
	for(int t=0;t<=T_max;t++)
	{
		for(int c=0;c<aircrafts.size();c++)
		{
			//skip retrofitted aircrafts at t=0 				& retrofitted types if retrofit is not available jet 																				& types which are not available jet
			if((aircrafts[c].is_retrofitted() && t<1) || (aircrafts[c].is_retrofitted() && t <= aircrafts[c].get_retro_purchasable_since_period()) || t < aircrafts[c].get_purchasable_since_period()) continue;

			//edges for purchasing aircrafts
			if(!aircrafts[c].is_retrofitted() && t<T_max)
			{
				temp_edge = boost::add_edge(p_market_node, node_list[i{t+1,c,0}], graph);
				if(temp_edge.second==true)
				{
					boost::put(edge_type,temp_edge.first,'p');
				}
				else
				{
					std::cout << "=======================\n";
					std::cout << "ERROR in constructor of fleet_graph: \nadding purchase edge not possible.\n";
					std::cout << "=======================\n";
				}
			}

			int service_t = aircrafts[c].get_service_t()+1;
			if(service_t > t-aircrafts[c].get_purchasable_since_period()) service_t = t-aircrafts[c].get_purchasable_since_period();
			for(int a=0;a<service_t;a++)
			{
				//skip types at service_t if available during optimization process
				if(aircrafts[c].get_purchasable_since_period() > 0 && a == service_t) continue;
				//skip retrofitted aircrafts at purchase age
				if(aircrafts[c].is_retrofitted() && a==0)	continue;
				//edges for selling aircrafts, no selling when aircraft was just bought
				if(a != 0)
				{
					temp_edge = boost::add_edge(node_list[i{t,c,a}], r_market_node, graph);
					if(temp_edge.second==true)
					{
						if(t != T_max)
						{
							boost::put(edge_type,temp_edge.first,'s');
						}
						else
						{
							//filler edges to get a nice graph picture
							boost::put(edge_type,temp_edge.first,'f');
						}
					}
					else
					{
						std::cout << "=======================\n";
						std::cout << "ERROR in constructor of fleet_graph: \nadding retail edge not possible.\n";
						std::cout << "=======================\n";
					}
				}
				else if( a == 0 && t == 0)
				{
					temp_edge=boost::add_edge(node_list[i{t,c,a}], r_market_node, graph);
					if(temp_edge.second==true)
					{
							//filler edges to get a nice graph picture
							boost::put(edge_type,temp_edge.first,'f');
					}
					else
					{
						std::cout << "=======================\n";
						std::cout << "ERROR in constructor of fleet_graph: \nadding invisible edge not possible.\n";
						std::cout << "=======================\n";
					}
				}

				if(t == T_max || a == aircrafts[c].get_service_t())
				{
					continue;
				}
				//iterate over all possible retrofit aim aircraft classes to create retrofitting or doing nothing edges
				int index=-1;
				for(auto c_=aircrafts[c].get_retrofit_aims().begin();c_!=aircrafts[c].get_retrofit_aims().end();c_++)
				{
					index++;
					if(aircrafts[*c_].is_retrofitted() && t < aircrafts[*c_].get_retro_purchasable_since_period()) continue;
					if(a>=service_t)	continue;
					//for retrofitted aircraft types is the age 1 in position 0 and 2 in pos 1 and so on
					temp_edge = boost::add_edge(node_list[i{t,c,a}], node_list[i{t+1,*c_,a+1}], graph);
					if(temp_edge.second==true)
					{
						if(*c_==c)
						{
							boost::put(edge_type,temp_edge.first,'o');
						}
						else
						{
							boost::put(edge_type,temp_edge.first,'r');
							boost::put(edge_cost,temp_edge.first,aircrafts[c].get_retrofit_price(index,t));
						}
					}
					else
					{
						std::cout << "=======================\n";
						std::cout << "ERROR in constructor of fleet_graph: \nadding retrofit or own edge not possible.\n";
						std::cout << "=======================\n";
					}
				}
			}
		}
	}
	std::cout << " DONE\n============\n";
}
