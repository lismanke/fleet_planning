#ifdef PROG_WITH_GRAPHVIZ
#include "fleet_graph_printer.hh"
using namespace std;

//creates a class to print the fleet planning graph if graphviz is installed
FleetGraphPrinter::FleetGraphPrinter(int t, int c, int n, int r)
{
  boost::filesystem::path dir_one(std::string("../output/") + date_namebuf.str() + "/" + std::to_string(t) + std::string("_periods/") + std::to_string(n) + std::string("_netclasses/") + std::to_string(c) + std::string("_base_aircraft_types/") + stringify_number(r) + std::string("_round/graph"));

  if(!(boost::filesystem::exists(dir_one)))
  {
    if (boost::filesystem::create_directory(dir_one))
    {
        std::cout << "folder graph successfully created!\n";
    }
  }
  graph_file_path = std::string("../output/") + date_namebuf.str() + "/" + std::to_string(t) + std::string("_periods/") + std::to_string(n) + std::string("_netclasses/") + std::to_string(c) + std::string("_base_aircraft_types/") + stringify_number(r) + std::string("_round/graph/graph");
}

//saves the grave inside a .dot file
void FleetGraphPrinter::print_dot_Graph(FleetGraph& fleet_graph)
{
  ofstream graph_file;
  graph_file.open((graph_file_path + ".dot").c_str(), ios::app);
  boost::write_graphviz(graph_file, fleet_graph.get_graph(), boost::make_label_writer(&create_vertex_names(fleet_graph)[0]), label_color_writer(fleet_graph.get_graph()));
  graph_file.close();
}

//saves a colored graph in a .dot file (solution for flow variables)
void FleetGraphPrinter::print_dot_Graph_solution(FleetGraph& fleet_graph, SCIP* scip, string file_suffix)
{
  ofstream graph_file;
  graph_file.open((graph_file_path + file_suffix + ".dot").c_str(), ios::app);
  boost::write_graphviz(graph_file, fleet_graph.get_graph(), boost::make_label_writer(&create_vertex_names(fleet_graph)[0]), label_color_writer_solution(fleet_graph.get_graph(), scip));
  graph_file.close();
}

//takes a .dot file and prints an actual picture of the graph
void FleetGraphPrinter::draw_Graph(string file_format, string file_suffix)
{
  GVC_t *gvc;
  Agraph_t *g;
  FILE *fp, *fp_out;
  gvc = gvContext();
  fp = fopen((graph_file_path + file_suffix + ".dot").c_str(), "r");
  fp_out = fopen((graph_file_path + file_suffix + "." + file_format).c_str(), "w");
  g = agread(fp, 0);
  gvLayout(gvc, g, "dot");
  gvRender(gvc, g, file_format.c_str(), fp_out);
  gvFreeLayout(gvc, g);
  agclose(g);
  fclose(fp);
  fclose(fp_out);
}

//function to name each vertex
vector<string> FleetGraphPrinter::create_vertex_names(FleetGraph& fleet_graph)
{
  Fleet_Planning_Graph::vertex_iterator vit, vend;
  vector<string> vertex_names;
  string namebuf;
  for(boost::tie(vit,vend)=boost::vertices(fleet_graph.get_graph());vit!=vend;vit++)
  {
    if(*vit == fleet_graph.get_p_market_node())
    {
      vertex_names.push_back("purchase_market");
    }
    else if(*vit == fleet_graph.get_r_market_node())
    {
      vertex_names.push_back("retail_market");
    }
    else
    {
      namebuf = "c:" + std::to_string(fleet_graph.get_vertex_class(*vit)) + ", a:" + std::to_string(fleet_graph.get_vertex_age(*vit)) + ", t:" + std::to_string(fleet_graph.get_vertex_time(*vit));
      vertex_names.push_back(namebuf);
    }
  }
  return(vertex_names);
}
#endif
