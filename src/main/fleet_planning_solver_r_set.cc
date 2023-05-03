#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <math.h>

#include <time.h>

#include <scip/scip.h>
#include <boost/program_options.hpp>

#include "data/data.hh"
#include "data/printer.hh"
#include "graph/fleet_graph.hh"

#ifdef PROG_WITH_GRAPHVIZ
#include "graph/fleet_graph_printer.hh"
#endif

#include "fleet_planning_program.hh"
#include "benders.hh"
#include "functions.hh"

#include "data/r_dataset.hh"

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
	bool graph_drawing;
	string graph_format;
	int range_start;
	int range;
	int types;
	int types_start;
	int T;
	int T_start;
	int T_solving;
	int agespan;
	int minage;
	int rounds;

	time_t my_time = time(NULL);

	po::options_description desc{"Options"};
	desc.add_options()("help,h", "Help screen")
										("graph_drawing", po::value<bool>(&graph_drawing)->default_value(false), "drawing for big instances is really slow")
										("graph_format", po::value<string>(&graph_format)->default_value("png"), "choose the preferred output format for the graph (png, jpeg,...)")
										("range", po::value<int>(&range)->default_value(15000), "maximum distance flown of airline")
										("range_start", po::value<int>(&range_start)->default_value(2000), "minimum distance flown of airline")
										("types", po::value<int>(&types)->default_value(25), "maximum number of aircraft types")
										("types_start", po::value<int>(&types_start)->default_value(2), "minimum number of aircraft types")
										("T", po::value<int>(&T)->default_value(30), "maximum considered periods")
										("T_start", po::value<int>(&T_start)->default_value(1), "minimum considered periods")
										("T_solving", po::value<int>(&T_solving)->default_value(3600), "maximum time for solving one instance")
										("agespan", po::value<int>(&agespan)->default_value(10), "span for maximum service time (ages) of the aircrafts")
										("minage", po::value<int>(&minage)->default_value(20), "minimum service time (ages) of the aircrafts")
										("rounds", po::value<int>(&rounds)->default_value(1), "number of random instances for each setup")
										("benders", po::value<bool>(&is_benders)->default_value(false), "solving instances by using benders decomposition");

	po::variables_map vm;
	po::positional_options_description positional_options;
	positional_options.add("graph_drawing", 1);
	positional_options.add("graph_format", 1);
	positional_options.add("range", 1);
	positional_options.add("range_start", 1);
	positional_options.add("types", 1);
	positional_options.add("types_start", 1);
	positional_options.add("T", 1);
	positional_options.add("T_start", 1);
	positional_options.add("T_solving", 1);
	positional_options.add("rounds", 1);
	positional_options.add("minage", 1);
	positional_options.add("agespan", 1);
	positional_options.add("benders", 1);

	po::store(po::command_line_parser(argc, argv)
								.options(desc)
								.positional(positional_options)
								.run(),
						vm);

	if (vm.count("help"))
	{
		std::cout << "Usage: "
							<< argv[0]
							<< " [options] <input> <ouput>"
							<< std::endl;

		std::cout << desc << std::endl;

		return 1;
	}

	po::notify(vm);
	std::cout << "maximum number of periods calculated T = " << T << " \n";

	// non-deterministic random numbers
	// std::random_device device;
	// std::mt19937 generator(device());

	Printer printer;

	T_starting = T_start;
	for (int t = T_start; t <= T; t++)
	{
		int n;
		for (int d = range_start; d <= range; d = d + 1000)
		{
			Sizes sizes;

			if (sizes.netclasses == round(sqrt(d / 42)) - 2)
				continue;
			sizes.netclasses = round(sqrt(d / 42)) - 2;
			for (int c = types_start; c <= types; c++)
			{
				sizes.age_range = {minage, minage + agespan};
				sizes.types = c;

				for (int r = 0; r < rounds; r++)
				{
					// deterministic random numbers
					std::mt19937 generator(40+r+t);
					std::cout << "=============== T: " << t << " types: " << c << " max_distance: " << d << " round: " << r + 1 << " ===============\n";
					int pos = 0;

					bool      orig_opt    = false;
					bool      benders_opt = false;
					SCIP_Real orig_obj    = 0.;
					SCIP_Real benders_obj = 0.;

					RandDataset rset(sizes, t, generator, d);
					Instance instance(rset, t);

					n = instance.get_net_classes().size();
					print_dataset(t, c, n, r, instance);

					FleetGraph graph(t, instance.get_market(), instance.get_aircrafts(), instance.get_net_classes());

					#ifdef PROG_WITH_GRAPHVIZ
						FleetGraphPrinter GraphPrinter(t, c, n, r);
						GraphPrinter.print_dot_Graph(graph);
						if (graph_drawing)
							GraphPrinter.draw_Graph(graph_format);
					#endif

					{
						OriginalProgram orig_program(graph, instance);

						print_scip_problem(orig_program.get_scip(), "orig");

						my_time = time(NULL);
						std::cout << "Solving orig program... Start:" << ctime(&my_time);
						solve_scip_problem(orig_program.get_scip(), T_solving);
						my_time = time(NULL);
						std::cout << "DONE! End:" << ctime(&my_time) << "\n";

						if (SCIPgetStatus(orig_program.get_scip()) == SCIP_STATUS_OPTIMAL)
						{
							orig_opt = true;
							orig_obj = SCIPgetPrimalbound(orig_program.get_scip());

							print_scip_solution(orig_program.get_scip(), "orig");

							#ifdef PROG_WITH_GRAPHVIZ
								GraphPrinter.print_dot_Graph_solution(orig_program.get_fleet_graph(), orig_program.get_scip(), "_orig_solution");
								if (graph_drawing)
									GraphPrinter.draw_Graph(graph_format, "_orig_solution");
							#endif
						}

						std::cout << "orig_obj is " << orig_obj << "\n\n";

						printer.print_stats(orig_program.get_scip(), t, c, n, d, r, false, is_benders);
						pos++;
					}

					if (is_benders)
					{
						BendersDecomp benders_program(graph, instance);

						// print problem
						print_scip_problem(benders_program.get_scip(), "master");

						my_time = time(NULL);
						std::cout << "Solving benders program... Start:" << ctime(&my_time);
						solve_scip_problem(benders_program.get_scip(), T_solving);
						my_time = time(NULL);

						std::cout << "DONE! End:" << ctime(&my_time) << "\n";

						if (SCIPgetStatus(benders_program.get_scip()) == SCIP_STATUS_OPTIMAL)
						{
							benders_opt = true;
							benders_obj = SCIPgetPrimalbound(benders_program.get_scip());

							// print solution when feasible
							print_scip_solution(benders_program.get_scip(), "master");
							
							#ifdef PROG_WITH_GRAPHVIZ
								GraphPrinter.print_dot_Graph_solution(benders_program.get_fleet_graph(), benders_program.get_scip(), "_benders_solution");
								if (graph_drawing)
									GraphPrinter.draw_Graph(graph_format, "_benders_solution");
							#endif
						}

						print_benders_statistics(benders_program.get_scip(), "benders_statistics");
						std::cout << "benders_obj is " << benders_obj << "\n\n";
						printer.print_stats(benders_program.get_scip(), t, c, n, d, r, true, is_benders);
						pos++;
					}
				}
			}
		}
	}
	return 0;
}
