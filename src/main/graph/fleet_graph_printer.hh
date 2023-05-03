#ifndef GRAPH_PRINTER_HH
#define GRAPH_PRINTER_HH

#ifdef PROG_WITH_GRAPHVIZ
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <stdio.h>

#include "boost/filesystem.hpp"
#include <graphviz/gvc.h>
#include <boost/graph/graphviz.hpp>

#include "../data/global.hh"
#include "../data/printer.hh"
#include "fleet_graph.hh"

using namespace std;

class FleetGraphPrinter{

  private:
    string graph_file_path;

  public:
    FleetGraphPrinter(int t, int c, int n, int r);

    //prints the Fleet_Planning_Graph
    void print_dot_Graph(FleetGraph& fleet_graph);

    void print_dot_Graph_solution(FleetGraph& fleet_graph, SCIP* scip, string file_suffix);

    void draw_Graph(string file_format, string file_suffix = "");

    std::vector<string> create_vertex_names(FleetGraph& fleet_graph);
};

//  define a property writer to color the edges as required
class label_color_writer {
  public:
    // constructor - needs reference to graph we are coloring
    label_color_writer(Fleet_Planning_Graph& g): myGraph(g){}
    // functor that does the coloring
    template <class VertexOrEdge>
    void operator()(std::ostream& out, const VertexOrEdge& edge) const
    {
      property_map<Fleet_Planning_Graph, edge_type_t>::type edge_type = boost::get(edge_type_t(), myGraph);

      if(get(edge_type, edge) == 'f')
      {
        out << "[color = white]";
      }
      else
      {
        out << "[label = " + to_string(get(edge_type, edge)) + " ]";
      }
    }
  private:
    Fleet_Planning_Graph& myGraph;
};

//property writer for the edgae labels when printing the solution
class label_color_writer_solution {
  public:
    // constructor - needs reference to graph we are coloring
    label_color_writer_solution(Fleet_Planning_Graph& g, SCIP* s): myGraph(g), scip(s){}
    // functor that does the coloring
    template <class VertexOrEdge>
    void operator()(std::ostream& out, const VertexOrEdge& edge) const
    {
      property_map<Fleet_Planning_Graph, edge_type_t>::type edge_type = boost::get(edge_type_t(), myGraph);
      property_map<Fleet_Planning_Graph, edge_var_t>::type edge_var = boost::get(edge_var_t(), myGraph);
      SCIP_SOL* sol = SCIPgetBestSol(scip);

      if(boost::get(edge_type, edge) == 'f')
      {
        out << "[color = white]";
      }
      else
      {
        if (SCIPgetSolVal(scip, sol, boost::get(edge_var, edge)) > 1e-06)
        {
          out << "[label=\"" << to_string(boost::get(edge_type, edge)) << "=" << SCIPgetSolVal(scip, sol, boost::get(edge_var, edge)) << "\", color = red]";
        }
        else
        {
          out << "[label=" << to_string(boost::get(edge_type, edge)) << "]";
        }
      }
    }
  private:
    Fleet_Planning_Graph& myGraph;
    SCIP* scip;
};

#endif
#endif
