#include "cap.h"
#include "trace.h"

#include <vector>
#include <iostream>
#include <fstream>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

using tr_vertex_t = ADDRINT;
using tr_graph_t =  boost::adjacency_list<boost::listS,
                                          boost::vecS,
                                          boost::bidirectionalS,
                                          tr_vertex_t>;

using tr_vertex_desc_t = tr_graph_t::vertex_descriptor;
using tr_edge_desc_t = tr_graph_t::edge_descriptor;
using tr_vertex_iter_t = tr_graph_t::vertex_iterator;
using tr_edge_iter_t = tr_graph_t::edge_iterator;

using bb_vertex_t = std::vector<ADDRINT>;
using bb_graph_t = boost::adjacency_list<boost::listS,
                                         boost::vecS,
                                         boost::bidirectionalS,
                                         bb_vertex_t>;

using bb_vertex_desc_t = bb_graph_t::vertex_descriptor;
using bb_edge_desc_t = bb_graph_t::edge_descriptor;
using bb_vertex_iter_t = bb_graph_t::vertex_iterator;
using bb_edge_iter_t = bb_graph_t::edge_iterator;

static tr_graph_t internal_graph;
static bb_graph_t internal_bb_graph;

auto construct_graph_from_trace () -> void
{
  auto find_vertex = [](tr_vertex_t vertex_value) -> tr_vertex_iter_t {
    auto first_vertex_iter = tr_vertex_iter_t();
    auto last_vertex_iter = tr_vertex_iter_t();
    std::tie(first_vertex_iter, last_vertex_iter) = boost::vertices(internal_graph);

    auto found_vertex_iter = last_vertex_iter;
    for (found_vertex_iter = first_vertex_iter; found_vertex_iter != last_vertex_iter; ++found_vertex_iter) {
      if (internal_graph[*found_vertex_iter] == vertex_value) break;
    }

    return found_vertex_iter;
  };

  auto prev_vertex_desc = tr_graph_t::null_vertex();
  for (const auto& inst : trace) {
    auto ins_addr = std::get<INS_ADDRESS>(inst);

    auto curr_vertex_iter = find_vertex(ins_addr);
    auto curr_vertex_desc = tr_vertex_desc_t();

    if (curr_vertex_iter == std::get<1>(boost::vertices(internal_graph))) {
      curr_vertex_desc = boost::add_vertex(ins_addr, internal_graph);
    }
    else curr_vertex_desc = *curr_vertex_iter;

    if (prev_vertex_desc != tr_graph_t::null_vertex()) {
      if (!std::get<1>(boost::edge(prev_vertex_desc, curr_vertex_desc, internal_graph))) {
        boost::add_edge(prev_vertex_desc, curr_vertex_desc, internal_graph);
      }
    }

    prev_vertex_desc = curr_vertex_desc;
  }

  return;
}


auto is_loopback_vertex(bb_vertex_desc_t vertex) -> bool
{
  auto first_out_edge_iter = bb_graph_t::out_edge_iterator();
  auto last_out_edge_iter = bb_graph_t::out_edge_iterator();

  std::tie(first_out_edge_iter, last_out_edge_iter) = boost::out_edges(vertex, internal_bb_graph);

  return std::any_of(first_out_edge_iter, last_out_edge_iter, [&vertex](bb_edge_desc_t edge_desc)
  {
    return (boost::target(edge_desc, internal_bb_graph) == vertex);
  });
}


static auto find_pivot_vertex () -> bb_vertex_desc_t
{
  auto first_vertex_iter = bb_vertex_iter_t();
  auto last_vertex_iter = bb_vertex_iter_t();
  std::tie(first_vertex_iter, last_vertex_iter) = boost::vertices(internal_bb_graph);

  auto found_vertex_iter = std::find_if(first_vertex_iter, last_vertex_iter, [](bb_vertex_desc_t vertex_desc)
  {
    if (!is_loopback_vertex(vertex_desc)) {
      auto in_degree = boost::in_degree(vertex_desc, internal_bb_graph);
      auto out_degree = boost::out_degree(vertex_desc, internal_bb_graph);

      if (out_degree == 1) {
        auto out_edge_iter = bb_graph_t::out_edge_iterator();
        std::tie(out_edge_iter, std::ignore) = boost::out_edges(vertex_desc, internal_bb_graph);

        auto next_vertex = boost::target(*out_edge_iter, internal_bb_graph);

        if ((boost::in_degree(next_vertex, internal_bb_graph) == 1) && !is_loopback_vertex(next_vertex)) {
          if ((in_degree == 0) || (in_degree >= 2)) {
            tfm::printfln("pivot found");
            return true;
          }
          else {
            auto in_edge_iter = bb_graph_t::in_edge_iterator();
            std::tie(in_edge_iter, std::ignore) = boost::in_edges(vertex_desc, internal_bb_graph);

            auto prev_vertex = boost::source(*in_edge_iter, internal_bb_graph);
            if (boost::out_degree(prev_vertex, internal_bb_graph) >= 2)
              tfm::printfln("pivot found s");
            return is_loopback_vertex(prev_vertex) || (boost::out_degree(prev_vertex, internal_bb_graph) >= 2);
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


static auto compress_graph_from_pivot_vertex (bb_vertex_desc_t pivot_vertex) -> void
{
  auto out_edge_iter = bb_graph_t::out_edge_iterator();
  std::tie(out_edge_iter, std::ignore) = boost::out_edges(pivot_vertex, internal_bb_graph);

  auto next_vertex = boost::target(*out_edge_iter, internal_bb_graph);

  boost::remove_out_edge_if(pivot_vertex, [](bb_edge_desc_t edge_desc) { return true; }, internal_bb_graph);

  internal_bb_graph[pivot_vertex].insert(
        std::end(internal_bb_graph[pivot_vertex]), std::begin(internal_bb_graph[next_vertex]), std::end(internal_bb_graph[next_vertex])
        );

  auto last_out_edge_iter = bb_graph_t::out_edge_iterator();
  std::tie(out_edge_iter, last_out_edge_iter) = boost::out_edges(next_vertex, internal_bb_graph);

  for (auto edge_iter = out_edge_iter; edge_iter != last_out_edge_iter; ++edge_iter) {
    auto next_next_vertex = boost::target(*edge_iter, internal_bb_graph);
    boost::add_edge(pivot_vertex, next_next_vertex, internal_bb_graph);
//    boost::remove_edge(edge_iter, internal_bb_graph);
  }
  boost::remove_out_edge_if(next_vertex, [](bb_edge_desc_t edge_desc) { return true; }, internal_bb_graph);

  boost::remove_vertex(next_vertex, internal_bb_graph);
  tfm::printfln("compress");

  return;
}


static auto get_tr_vertex_desc(tr_vertex_t addr)  -> tr_vertex_desc_t
{
  auto first_vertex_iter = tr_vertex_iter_t();
  auto last_vertex_iter = tr_vertex_iter_t();

  std::tie(first_vertex_iter, last_vertex_iter) = boost::vertices(internal_graph);
  for (auto vertex_iter = first_vertex_iter; vertex_iter != last_vertex_iter; ++vertex_iter) {
    if (internal_graph[*vertex_iter] == addr) return *vertex_iter;
  }
  return tr_graph_t::null_vertex();
}


static auto construct_bb_graph () -> void
{
  auto first_vertex_iter = tr_vertex_iter_t();
  auto last_vertex_iter = tr_vertex_iter_t();

  std::tie(first_vertex_iter, last_vertex_iter) = boost::vertices(internal_graph);
  for (auto vertex_iter = first_vertex_iter; vertex_iter != last_vertex_iter; ++vertex_iter) {
    boost::add_vertex(bb_vertex_t{internal_graph[*vertex_iter]}, internal_bb_graph);
  }

  auto first_bb_vertex_iter = bb_vertex_iter_t();
  auto last_bb_vertex_iter = bb_vertex_iter_t();

  std::tie(first_bb_vertex_iter, last_bb_vertex_iter) = boost::vertices(internal_bb_graph);

  for (auto src_bb_vertex_iter = first_bb_vertex_iter; src_bb_vertex_iter != last_bb_vertex_iter; ++src_bb_vertex_iter)
    for (auto dst_bb_vertex_iter = first_bb_vertex_iter; dst_bb_vertex_iter != last_bb_vertex_iter; ++dst_bb_vertex_iter) {

      auto src_tr_vertex_desc = get_tr_vertex_desc(internal_bb_graph[*src_bb_vertex_iter].front());
      auto dst_tr_vertex_desc = get_tr_vertex_desc(internal_bb_graph[*dst_bb_vertex_iter].front());

      if (std::get<1>(boost::edge(src_tr_vertex_desc, dst_tr_vertex_desc, internal_graph)) &&
          !std::get<1>(boost::edge(*src_bb_vertex_iter, *dst_bb_vertex_iter, internal_bb_graph))) {
        boost::add_edge(*src_bb_vertex_iter, *dst_bb_vertex_iter, internal_bb_graph);
      }
    }

  do {
    auto pivot_vertex_desc = find_pivot_vertex();
    if (pivot_vertex_desc != bb_graph_t::null_vertex()) {
      compress_graph_from_pivot_vertex(pivot_vertex_desc);
//      break;
    }
    else break;
  }
  while (true);


  return;
}


/*====================================================================================================================*/
/*                                                     exported functions                                             */
/*====================================================================================================================*/

auto cap_save_trace_to_dot_file (const std::string& filename) noexcept -> void
{
  construct_graph_from_trace();

  auto write_vertex = [](std::ostream& label, tr_vertex_desc_t vertex_desc) -> void {
    tfm::format(label, "[label=\"%s:%s\"]", StringFromAddrint(internal_graph[vertex_desc]),
                cached_ins_at_addr[internal_graph[vertex_desc]]->disassemble);
    return;
  };

  auto write_edge = [](std::ostream& label, tr_edge_desc_t edge_desc) -> void {
    tfm::format(label, "[label=\"\"]");
    return;
  };

  ofstream output_file(filename.c_str(), std::ofstream::out | std::ofstream::trunc);
  if (output_file.is_open()) {
    boost::write_graphviz(output_file, internal_graph,
                          std::bind(write_vertex, std::placeholders::_1, std::placeholders::_2),
                          std::bind(write_edge, std::placeholders::_1, std::placeholders::_2));
    output_file.close();
  }
  else {
    tfm::printfln("cannot save trace to dot file %s", filename);
  }

  return;
}

template<uint32_t align>
auto print_ins (std::ostream& label, std::string str1, std::string str2) -> void
{
//  tfm::format(label, "%-12s %-35s\\l", str1, str2);
  return;
}

template<>
auto print_ins<15> (std::ostream& label, std::string str1, std::string str2) -> void
{
  tfm::format(label, "%-12s %-15s\\l", str1, str2);
  return;
}

template<>
auto print_ins<20> (std::ostream& label, std::string str1, std::string str2) -> void
{
  tfm::format(label, "%-12s %-20s\\l", str1, str2);
  return;
}

template<>
auto print_ins<25> (std::ostream& label, std::string str1, std::string str2) -> void
{
  tfm::format(label, "%-12s %-25s\\l", str1, str2);
  return;
}

template<>
auto print_ins<30> (std::ostream& label, std::string str1, std::string str2) -> void
{
  tfm::format(label, "%-12s %-30s\\l", str1, str2);
  return;
}

template<>
auto print_ins<35> (std::ostream& label, std::string str1, std::string str2) -> void
{
  tfm::format(label, "%-12s %-35s\\l", str1, str2);
  return;
}

auto cap_save_basic_block_trace_to_dot_file (const std::string& filename) noexcept -> void
{
  construct_bb_graph();

  auto write_vertex = [](std::ostream& label, bb_vertex_desc_t vertex_desc) -> void {

    auto ins_disas_name_sizes = std::vector<uint32_t>(internal_bb_graph[vertex_desc].size());
    std::transform(std::begin(internal_bb_graph[vertex_desc]), std::end(internal_bb_graph[vertex_desc]),
                   std::begin(ins_disas_name_sizes), [](ADDRINT addr)
    {
      return (cached_ins_at_addr[addr]->disassemble).length();
    });

    auto name_max_size = *std::max_element(std::begin(ins_disas_name_sizes), std::end(ins_disas_name_sizes));
    tfm::printfln("max size = %d", name_max_size);

    if (boost::in_degree(vertex_desc, internal_bb_graph) == 0) {
      tfm::format(label, "[shape=box,style=\"filled,rounded\",fillcolor=cornflowerblue,label=\"");
    }
    else if (boost::out_degree(vertex_desc, internal_bb_graph) == 0) {
      tfm::format(label, "[shape=box,style=\"filled,rounded\",fillcolor=gainsboro,label=\"");
    }
    else if (boost::in_degree(vertex_desc, internal_bb_graph) > 2) {
      tfm::format(label, "[shape=box,style=\"filled,rounded\",fillcolor=darkorchid1,label=\"");
    }
    else if (boost::out_degree(vertex_desc, internal_bb_graph) > 2) {
      tfm::format(label, "[shape=box,style=\"filled,rounded\",fillcolor=darkgoldenrod1,label=\"");
    }
    else tfm::format(label, "[shape=box,style=rounded,label=\"");

    for (const auto& addr : internal_bb_graph[vertex_desc]) {
      /*if (std::addressof(addr) == std::addressof(internal_bb_graph[vertex_desc].back())) {
        tfm::format(label, "%-12s %-s", StringFromAddrint(addr), cached_ins_at_addr[addr]->disassemble);
      }
      else*/
      tfm::format(label, "%-12s %-s\\l", StringFromAddrint(addr), cached_ins_at_addr[addr]->disassemble);
    }
//    tfm::format(label, "\",fontname=Courier,fontsize=10.0]");
    tfm::format(label, "\",fontname=\"Inconsolata\",fontsize=10.0]");

    return;
  };

  auto write_edge = [](std::ostream& label, bb_edge_desc_t edge_desc) -> void {
    tfm::format(label, "[label=\"\"]");
    return;
  };

  ofstream output_file(filename.c_str(), std::ofstream::out | std::ofstream::trunc);
  if (output_file.is_open()) {
    boost::write_graphviz(output_file, internal_bb_graph,
                          std::bind(write_vertex, std::placeholders::_1, std::placeholders::_2),
                          std::bind(write_edge, std::placeholders::_1, std::placeholders::_2));
    output_file.close();
  }
  else {
    tfm::printfln("cannot save to file %s", filename);
  }

  return;
}
