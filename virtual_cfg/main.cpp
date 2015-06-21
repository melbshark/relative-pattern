#include "tinyformat.h"

#include <set>
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
static auto virtual_inss = std::set<tr_vertex_t>{};

typedef std::vector<tr_vertex_t> bb_vertex_t;
typedef std::vector<bb_vertex_t> bb_vertices_t;

typedef boost::adjacency_list<boost::listS,
                              boost::vecS,
                              boost::bidirectionalS,
                              bb_vertex_t> bb_graph_t;

typedef bb_graph_t::vertex_descriptor bb_vertex_desc_t;
typedef bb_graph_t::edge_descriptor bb_edge_desc_t;
typedef bb_graph_t::vertex_iterator bb_vertex_iter_t;
typedef bb_graph_t::edge_iterator bb_edge_iter_t;

static bb_graph_t virtual_bb_cfg;
static bb_vertex_desc_t root_bb_vertex_desc = bb_graph_t::null_vertex();

/*===========================================================================*/


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
      try {
        output_trace.push_back(std::stoul(srt));
      } catch (std::exception& expt) {
        tfm::printfln("%s", expt.what());
        break;
      }
    }
  }

//  auto virt_inss = std::set<uint32_t>{};
//  for (const auto& virt_ins : output_trace) {
//    virt_inss.insert(virt_ins);
//  }

//  tfm::printfln("trace length %d, of %d different instructions", output_trace.size(), virt_inss.size());

  return output_trace;
}


auto add_trace_into_cfg (const tr_vertices_t& trace) -> void
{
  if (!trace.empty()) {
    if (boost::num_vertices(virtual_cfg) == 0) {
      root_vertex_desc = boost::add_vertex(trace.front(), virtual_cfg);
    }

    assert(root_vertex_desc == trace.front());

    auto current_vertex_desc = root_vertex_desc;
    auto next_vertex_desc = tr_graph_t::null_vertex();

    std::for_each(std::begin(trace) + 1, std::end(trace), [&](uint32_t virt_addr) {
      auto first_out_edge_iter = tr_graph_t::out_edge_iterator();
      auto last_out_edge_iter = tr_graph_t::out_edge_iterator();

      std::tie(first_out_edge_iter, last_out_edge_iter) = boost::out_edges(current_vertex_desc, virtual_cfg);

      auto next_edge_iter = std::find_if(first_out_edge_iter, last_out_edge_iter, [&](tr_edge_desc_t edge_desc) {
        auto target_desc = boost::target(edge_desc, virtual_cfg);
        return (virtual_cfg[target_desc] == virt_addr);
      });

      if (next_edge_iter != last_out_edge_iter) {
        next_vertex_desc = boost::target(*next_edge_iter, virtual_cfg);
      }
      else {
        auto first_vertex_iter = tr_graph_t::vertex_iterator();
        auto last_vertex_iter = tr_graph_t::vertex_iterator();

        std::tie(first_vertex_iter, last_vertex_iter) = boost::vertices(virtual_cfg);
        auto next_vertex_iter = std::find_if(first_vertex_iter, last_vertex_iter, [&](tr_vertex_desc_t vertex_desc) {
          return (virtual_cfg[vertex_desc] == virt_addr);
        });

        if (next_vertex_iter != last_vertex_iter) {
          next_vertex_desc = *next_vertex_iter;
        }
        else next_vertex_desc = boost::add_vertex(virt_addr, virtual_cfg);

        boost::add_edge(current_vertex_desc, next_vertex_desc, virtual_cfg);
      }

      current_vertex_desc = next_vertex_desc;
    });
  }
  return;
}


auto collect_virtual_instructions () -> void
{
  auto first_vertex_iter = tr_vertex_iter_t();
  auto last_vertex_iter = tr_vertex_iter_t();

  std::tie(first_vertex_iter, last_vertex_iter) = boost::vertices(virtual_cfg);

  std::for_each(first_vertex_iter, last_vertex_iter, [&](tr_vertex_desc_t vertex_desc)
  {
    virtual_inss.insert(virtual_cfg[vertex_desc]);
  });

  return;
}


auto save_cfg_to_file (const std::string& filename) -> void
{
  if (boost::num_vertices(virtual_cfg) > 0) {
    auto write_vertex = [](std::ostream& label, tr_vertex_desc_t vertex_desc) -> void {
      tfm::format(label, "[shape=box,style=rounded,label=\"%d\"]", virtual_cfg[vertex_desc]);
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


auto is_loopback_bb (bb_vertex_desc_t vertex_desc) -> bool
{
  auto first_out_edge_iter = bb_graph_t::out_edge_iterator();
  auto last_out_edge_iter = bb_graph_t::out_edge_iterator();

  std::tie(first_out_edge_iter, last_out_edge_iter) = boost::out_edges(vertex_desc, virtual_bb_cfg);

  return std::any_of(first_out_edge_iter, last_out_edge_iter, [&](bb_edge_desc_t edge_desc)
  {
    return (boost::target(edge_desc, virtual_bb_cfg) == vertex_desc);
  });
}


auto is_first_bb (bb_vertex_desc_t vertex_desc) -> bool
{
  return (virtual_bb_cfg[vertex_desc].front() == virtual_cfg[root_vertex_desc]);
}


auto find_pivot_bb () -> bb_vertex_desc_t
{
  auto first_vertex_iter = bb_vertex_iter_t();
   auto last_vertex_iter  = bb_vertex_iter_t();
   std::tie(first_vertex_iter, last_vertex_iter) = boost::vertices(virtual_bb_cfg);

   auto found_vertex_iter = std::find_if(first_vertex_iter, last_vertex_iter, [](bb_vertex_desc_t current_vertex)
   {
     if (!is_loopback_bb(current_vertex)) {
       auto in_degree = boost::in_degree(current_vertex, virtual_bb_cfg);
       auto out_degree = boost::out_degree(current_vertex, virtual_bb_cfg);

       if (out_degree == 1) {
         auto out_edge_iter = bb_graph_t::out_edge_iterator();
         std::tie(out_edge_iter, std::ignore) = boost::out_edges(current_vertex, virtual_bb_cfg);

         auto next_vertex = boost::target(*out_edge_iter, virtual_bb_cfg);

         if ((boost::in_degree(next_vertex, virtual_bb_cfg) == 1) && !is_loopback_bb(next_vertex) && !is_first_bb(next_vertex)) {
           if ((in_degree == 0) || (in_degree >= 2)) {
 //            tfm::printfln("pivot found");
             return true;
           }
           else {
             auto in_edge_iter = bb_graph_t::in_edge_iterator();
             std::tie(in_edge_iter, std::ignore) = boost::in_edges(current_vertex, virtual_bb_cfg);

             auto prev_vertex = boost::source(*in_edge_iter, virtual_bb_cfg);
             if (boost::out_degree(prev_vertex, virtual_bb_cfg) >= 2)
 //              tfm::printfln("pivot found s");
               return (is_loopback_bb(prev_vertex) || (boost::out_degree(prev_vertex, virtual_bb_cfg) >= 2));
             else return is_first_bb(current_vertex);
           }
         }
         else return false;
       }
       else return false;
     }
     return false;
   });

   if (found_vertex_iter != last_vertex_iter) return *found_vertex_iter;
   else return bb_graph_t::null_vertex();
}


auto compress_bb_cfg_from_pivot_vertex (bb_vertex_desc_t pivot_vertex) -> void
{
  auto out_edge_iter = bb_graph_t::out_edge_iterator();
  std::tie(out_edge_iter, std::ignore) = boost::out_edges(pivot_vertex, virtual_bb_cfg);

  auto next_vertex = boost::target(*out_edge_iter, virtual_bb_cfg);

  boost::remove_out_edge_if(pivot_vertex, [](bb_edge_desc_t edge_desc) { return true; }, virtual_bb_cfg);

  virtual_bb_cfg[pivot_vertex].insert(
        std::end(virtual_bb_cfg[pivot_vertex]),
        std::begin(virtual_bb_cfg[next_vertex]), std::end(virtual_bb_cfg[next_vertex])
        );


  auto last_out_edge_iter = bb_graph_t::out_edge_iterator();
  std::tie(out_edge_iter, last_out_edge_iter) = boost::out_edges(next_vertex, virtual_bb_cfg);

  for (auto edge_iter = out_edge_iter; edge_iter != last_out_edge_iter; ++edge_iter) {
    auto next_next_vertex = boost::target(*edge_iter, virtual_bb_cfg);
    boost::add_edge(pivot_vertex, next_next_vertex, virtual_bb_cfg);
  }
  boost::remove_out_edge_if(next_vertex, [](bb_edge_desc_t edge_desc) { return true; }, virtual_bb_cfg);

  boost::remove_vertex(next_vertex, virtual_bb_cfg);

  return;
}


auto construct_virtual_bb_cfg () -> void
{
  auto first_vertex_iter = tr_vertex_iter_t();
   auto last_vertex_iter = tr_vertex_iter_t();

   std::tie(first_vertex_iter, last_vertex_iter) = boost::vertices(virtual_cfg);
   for (auto vertex_iter = first_vertex_iter; vertex_iter != last_vertex_iter; ++vertex_iter) {
     boost::add_vertex(tr_vertices_t{virtual_cfg[*vertex_iter]}, virtual_bb_cfg);
   }

   auto first_bb_vertex_iter = bb_vertex_iter_t();
   auto last_bb_vertex_iter = bb_vertex_iter_t();

   std::tie(first_bb_vertex_iter, last_bb_vertex_iter) = boost::vertices(virtual_bb_cfg);

   auto get_tr_vertex_desc = [](tr_vertex_t addr)  -> tr_vertex_desc_t
   {
     auto first_vertex_iter = tr_vertex_iter_t();
     auto last_vertex_iter = tr_vertex_iter_t();

     std::tie(first_vertex_iter, last_vertex_iter) = boost::vertices(virtual_cfg);
     for (auto vertex_iter = first_vertex_iter; vertex_iter != last_vertex_iter; ++vertex_iter) {
       if (virtual_cfg[*vertex_iter] == addr) return *vertex_iter;
     }
     return tr_graph_t::null_vertex();
   };

   for (auto src_bb_vertex_iter = first_bb_vertex_iter; src_bb_vertex_iter != last_bb_vertex_iter; ++src_bb_vertex_iter)
     for (auto dst_bb_vertex_iter = first_bb_vertex_iter; dst_bb_vertex_iter != last_bb_vertex_iter; ++dst_bb_vertex_iter) {

       auto src_tr_vertex_desc = get_tr_vertex_desc(virtual_bb_cfg[*src_bb_vertex_iter].front());
       auto dst_tr_vertex_desc = get_tr_vertex_desc(virtual_bb_cfg[*dst_bb_vertex_iter].front());

       if (std::get<1>(boost::edge(src_tr_vertex_desc, dst_tr_vertex_desc, virtual_cfg)) &&
           !std::get<1>(boost::edge(*src_bb_vertex_iter, *dst_bb_vertex_iter, virtual_bb_cfg))) {
         boost::add_edge(*src_bb_vertex_iter, *dst_bb_vertex_iter, virtual_bb_cfg);
       }
     }

   do {
     auto pivot_vertex_desc = find_pivot_bb();
     if (pivot_vertex_desc != bb_graph_t::null_vertex()) {
       compress_bb_cfg_from_pivot_vertex(pivot_vertex_desc);
     }
     else break;
   }
   while (true);

  return;
}


auto save_bb_cfg_to_file (const std::string& filename) -> void
{
  if (boost::num_vertices(virtual_bb_cfg) > 0) {
    auto write_vertex = [](std::ostream& label, bb_vertex_desc_t vertex_desc) -> void {
      if (is_first_bb(vertex_desc)) {
        tfm::format(label, "[shape=box,style=\"filled,rounded\",fillcolor=cornflowerblue,label=\"");
      }
      else if (boost::out_degree(vertex_desc, virtual_bb_cfg) == 0) {
        tfm::format(label, "[shape=box,style=\"filled,rounded\",fillcolor=gainsboro,label=\"");
      }
      else tfm::format(label, "[shape=box,style=rounded,label=\"");

      for (const auto& virt_ins : virtual_bb_cfg[vertex_desc]) {
        tfm::format(label, "%d ", virt_ins);
      }

      tfm::format(label, "\",fontname=\"Inconsolata\",fontsize=10.0]");
      return;
    };

    auto write_edge = [](std::ostream& label, bb_edge_desc_t edge_desc) -> void {
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


/*===========================================================================*/

int main(int argc, char* argv[])
{
  if (argc > 3) {
    auto ins_number = uint32_t{0};

    for (int32_t i = 1; i < argc - 2; ++i) {
      auto new_trace = get_virtual_trace(std::string(argv[i]));
      add_trace_into_cfg(new_trace);
      ins_number += new_trace.size();
    }

//    collect_virtual_instructions();

    tfm::printfln("total trace length: %d", ins_number);
    tfm::printfln("traced virtual instructions: %d", boost::num_vertices(virtual_cfg));
//    tfm::printfln("different virtual instructions: %d", virtual_inss.size());

    tfm::printfln("save virtual graph to file: %s", argv[argc - 2]);
    save_cfg_to_file(std::string(argv[argc - 2]));

    construct_virtual_bb_cfg();
    tfm::printfln("save virtual basic block graph to file: %s", argv[argc - 1]);
    save_bb_cfg_to_file(std::string(argv[argc - 1]));
  }

  return 0;
}

