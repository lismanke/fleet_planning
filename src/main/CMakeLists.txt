set(COMMON_SRC
  data/r_dataset.cc
  data/global.cc
  data/printer.cc
  graph/fleet_graph_printer.cc
  graph/fleet_graph.cc
  functions_for_vars.cc
  functions_for_cons.cc
  functions_for_decomp.cc
  fleet_planning_program.cc
  benders.cc)

add_library(common ${COMMON_SRC})

set(libs
  ${libs}
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
  ${SCIP_LIBRARIES})

if(GRAPHVIZ_FOUND)
  set(libs
    ${libs}
    ${GRAPHVIZ_GVC_LIBRARY}
    ${GRAPHVIZ_CDT_LIBRARY}
    ${GRAPHVIZ_CGRAPH_LIBRARY}
    ${GRAPHVIZ_PATHPLAN_LIBRARY})
endif()

target_link_libraries(common ${libs})

add_executable(fleet_planning_solver_r_set fleet_planning_solver_r_set.cc)

target_link_libraries(fleet_planning_solver_r_set common)

