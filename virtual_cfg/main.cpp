#include "tinyformat.h"

#include <vector>
#include <cstdint>
#include <fstream>
#include <string>
#include <algorithm>

#include <boost/algorithm/string.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/graphviz.hpp>

typedef uint32_t tr_vertex_t;
typedef std::vector<tr_vertex_t> tr_vertices_t;

typedef boost::adjacency_list<boost::listS,
                              boost::vecS,
                              boost::bidirectionalS,
                              tr_vertex_t> tr_graph_t;

typedef tr_graph_t::vertex_descriptor tr_vertex_desc_t;
typedef tr_graph_t::edge_descriptor tr_edge_desc_t;
typedef tr_graph_t::vertex_iterator tr_vertex_iter_t;
typedef tr_graph_t::edge_iterator tr_edge_iter_t;

static tr_graph_t virtual_cfg;
static tr_vertex_desc_t root_vertex_desc = tr_graph_t::null_vertex();

auto get_virtual_trace (const std::string& filename) -> tr_vertices_t
{
  auto input_file = std::ifstream(filename.c_str());
  auto output_trace = tr_vertices_t{};

  if (input_file.is_open()) {
    auto line = std::string();
    std::getline(input_file, line);

    auto strs = std::vector<std::string>{};
    boost::split(strs, line, boost::is_any_of(", "), boost::token_compress_on);

    for (auto& srt : strs) {
      output_trace.push_back(std::stoul(srt, 0, 10));
    }
  }

  return output_trace;
}


auto add_trace_into_cfg (const tr_vertices_t& trace) -> void
{
  if (!trace.empty()) {
    auto offset = uint32_t{0};
    if (boost::num_vertices(virtual_cfg) == 0) {
      root_vertex_desc = boost::add_vertex(trace.front(), virtual_cfg);
      offset = 1;
    }
    else {
      assert(root_vertex_desc == trace.front());
      
//      auto current_virt_in_trace = trace.front();
      auto current_vertex_desc = root_vertex_desc;
      
      std::for_each(std::begin(trace) + 1, std::end(trace), [&](uint32_t virt_addr) {
        auto first_out_edge_iter = tr_graph_t::out_edge_iterator();
        auto last_out_edge_iter = tr_graph_t::out_edge_iterator();
        
        std::tie(first_out_edge_iter, last_out_edge_iter) = boost::out_edges(current_vertex_desc, virtual_cfg);
        
        auto next_edge_iter = std::find_if(first_out_edge_iter, last_out_edge_iter, [&](tr_edge_desc_t edge_desc) {
          auto target_desc = boost::target(edge_desc, virtual_cfg);
          return (virtual_cfg[target_desc] == virt_addr);
        });
                   
        if (next_edge_iter != last_out_edge_iter) {
          current_vertex_desc = boost::target(*next_edge_iter, virtual_cfg);
        }
        else {
          current_vertex_desc = boost::add_vertex(virt_addr, virtual_cfg);
        }
      });
    }
  }
  return;
}

auto save_cfg_to_file (const std::string& filename) -> void
{
  if (boost::num_vertices(virtual_cfg) > 0) {
    auto write_vertex = [](std::ostream& label, tr_vertex_desc_t vertex_desc) -> void {
      tfm::format(label, "[label=\"%d\"]", virtual_cfg[vertex_desc]);
      return;
    };

    auto write_edge = [](std::ostream& label, tr_edge_desc_t edge_desc) -> void {
      tfm::format(label, "[label=\"\"]");
      return;
    };

    auto output_file = std::ofstream(filename.c_str(), std::ofstream::trunc);
    if (output_file.is_open()) {
      boost::write_graphviz(output_file, virtual_cfg,
                            std::bind(write_vertex, std::placeholders::_1, std::placeholders::_2),
                            std::bind(write_edge, std::placeholders::_1, std::placeholders::_2));
      output_file.close();
    }
  }
  else {
    tfm::printfln("virtual graph empty");
  }

  return;
}

int main(int argc, char* argv[])
{
  if (argc > 2) {
    for (int32_t i = 1; i < argc - 1; ++i) {
      auto new_trace = get_virtual_trace(std::string(argv[i]));
      add_trace_into_cfg(new_trace);
    }

    save_cfg_to_file(std::string(argv[argc - 1]));
  }

  return 0;
}

