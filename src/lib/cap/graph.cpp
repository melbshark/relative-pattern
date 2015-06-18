#include "cap.h"
#include "trace.h"

#include <vector>
#include <queue>
#include <list>
#include <iostream>
#include <fstream>
#include <algorithm>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/graphviz.hpp>
//#include <boost/graph/graph_utility.hpp>

//#include <boost/graph/adj_list_serialize.hpp>
//#include <boost/archive/binary_iarchive.hpp>
//#include <boost/archive/binary_oarchive.hpp>


using tr_vertex_t   = ADDRINT;
using tr_vertices_t = std::vector<tr_vertex_t>;
using tr_graph_t    =  boost::adjacency_list<boost::listS,
                                             boost::vecS,
                                             boost::bidirectionalS,
                                             tr_vertex_t>;

using tr_vertex_desc_t = tr_graph_t::vertex_descriptor;
using tr_edge_desc_t   = tr_graph_t::edge_descriptor;
using tr_vertex_iter_t = tr_graph_t::vertex_iterator;
using tr_edge_iter_t   = tr_graph_t::edge_iterator;

//using bb_vertex_t = std::vector<ADDRINT>;
using bb_vertex_t = std::pair<int32_t, tr_vertices_t>;
enum
{
  BB_ORDER = 0,
  BB_ADDRESSES = 1
};

using bb_graph_t = boost::adjacency_list<boost::listS,
                                         boost::vecS,
                                         boost::bidirectionalS,
                                         bb_vertex_t>;

using bb_vertex_desc_t = bb_graph_t::vertex_descriptor;
using bb_edge_desc_t   = bb_graph_t::edge_descriptor;
using bb_vertex_iter_t = bb_graph_t::vertex_iterator;
using bb_edge_iter_t   = bb_graph_t::edge_iterator;

using bbs_vertex_t = std::vector<int32_t>;
using bbs_graph_t = boost::adjacency_list<boost::listS,
                                          boost::vecS,
                                          boost::bidirectionalS,
                                          bbs_vertex_t>;
using bbs_vertex_desc_t = bbs_graph_t::vertex_descriptor;
using bbs_edge_desc_t = bbs_graph_t::edge_descriptor;
using bbs_vertex_iter_t = bb_graph_t::vertex_iterator;
using bb_edge_iter_t = bb_graph_t::edge_iterator;

using bbss_vertex_t = std::vector<bbs_vertex_t>;
using bbss_graph_t = boost::adjacency_list<boost::listS, boost::vecS, boost::bidirectionalS, bbss_vertex_t>;
using bbss_vertex_desc_t = bbss_graph_t::vertex_descriptor;
using bbss_edge_desc_t = bbss_graph_t::edge_descriptor;
using bbss_vertex_iter_t = bbss_graph_t::vertex_iterator;
using bbss_edge_iter_t = bbss_graph_t::edge_iterator;

static tr_graph_t internal_graph;
static bb_graph_t internal_bb_graph;
static bbs_graph_t internal_bbs_graph;
static bbss_graph_t internal_bbss_graph;

auto normalize_hex_string (const std::string& input) -> std::string
{
  assert(input.find("0x") == 0);
  auto first_non_zero_iter = std::find_if(std::begin(input) + 2, std::end(input), [](char c) { return (c != '0');});
  auto output = first_non_zero_iter != std::end(input) ? std::string(first_non_zero_iter, std::end(input)) : std::string("0");
  return ("0x" + output);
}

auto construct_graph_from_trace () -> void
{
  auto find_vertex = [](tr_vertex_t vertex_value) -> tr_vertex_iter_t {
    auto first_vertex_iter = tr_vertex_iter_t();
    auto last_vertex_iter  = tr_vertex_iter_t();
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
  auto last_out_edge_iter  = bb_graph_t::out_edge_iterator();

  std::tie(first_out_edge_iter, last_out_edge_iter) = boost::out_edges(vertex, internal_bb_graph);

  return std::any_of(first_out_edge_iter, last_out_edge_iter, [&vertex](bb_edge_desc_t edge_desc)
  {
    return (boost::target(edge_desc, internal_bb_graph) == vertex);
  });
}


static auto is_first_bb (bb_vertex_desc_t vertex_desc) -> bool
{
  return (std::get<BB_ADDRESSES>(internal_bb_graph[vertex_desc]).front() == std::get<INS_ADDRESS>(trace.front()));
}


static auto find_pivot_vertex () -> bb_vertex_desc_t
{
  auto first_vertex_iter = bb_vertex_iter_t();
  auto last_vertex_iter  = bb_vertex_iter_t();
  std::tie(first_vertex_iter, last_vertex_iter) = boost::vertices(internal_bb_graph);

  auto found_vertex_iter = std::find_if(first_vertex_iter, last_vertex_iter, [](bb_vertex_desc_t current_vertex)
  {
    if (!is_loopback_vertex(current_vertex)) {
      auto in_degree = boost::in_degree(current_vertex, internal_bb_graph);
      auto out_degree = boost::out_degree(current_vertex, internal_bb_graph);

      if (out_degree == 1) {
        auto out_edge_iter = bb_graph_t::out_edge_iterator();
        std::tie(out_edge_iter, std::ignore) = boost::out_edges(current_vertex, internal_bb_graph);

        auto next_vertex = boost::target(*out_edge_iter, internal_bb_graph);

        if ((boost::in_degree(next_vertex, internal_bb_graph) == 1) && !is_loopback_vertex(next_vertex) && !is_first_bb(next_vertex)) {
          if ((in_degree == 0) || (in_degree >= 2)) {
//            tfm::printfln("pivot found");
            return true;
          }
          else {
            auto in_edge_iter = bb_graph_t::in_edge_iterator();
            std::tie(in_edge_iter, std::ignore) = boost::in_edges(current_vertex, internal_bb_graph);

            auto prev_vertex = boost::source(*in_edge_iter, internal_bb_graph);
            if (boost::out_degree(prev_vertex, internal_bb_graph) >= 2)
//              tfm::printfln("pivot found s");
              return (is_loopback_vertex(prev_vertex) || (boost::out_degree(prev_vertex, internal_bb_graph) >= 2));
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


static auto compress_graph_from_pivot_vertex (bb_vertex_desc_t pivot_vertex) -> void
{
  auto out_edge_iter = bb_graph_t::out_edge_iterator();
  std::tie(out_edge_iter, std::ignore) = boost::out_edges(pivot_vertex, internal_bb_graph);

  auto next_vertex = boost::target(*out_edge_iter, internal_bb_graph);

  boost::remove_out_edge_if(pivot_vertex, [](bb_edge_desc_t edge_desc) { return true; }, internal_bb_graph);

  std::get<BB_ADDRESSES>(internal_bb_graph[pivot_vertex]).insert(
        std::end(std::get<BB_ADDRESSES>(internal_bb_graph[pivot_vertex])),
        std::begin(std::get<BB_ADDRESSES>(internal_bb_graph[next_vertex])),
        std::end(std::get<BB_ADDRESSES>(internal_bb_graph[next_vertex]))
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
//  tfm::printfln("compress");

  return;
}


//static auto linear_node_order = int32_t{0};
static auto linear_visited_bbs = std::vector<bb_vertex_desc_t>{};
struct node_numbering_visitor : public boost::default_bfs_visitor
{
//  static std::vector<bb_vertex_desc_t> linear_visited_bbs;

  void discover_vertex(bb_vertex_desc_t vertex_desc, const bb_graph_t& graph)
  {
//    if (std::get<NODE_ORDER>(graph[vertex_desc]) == -1) {
//      linear_visited_bbs.push_back(vertex_desc);
//    }
    linear_visited_bbs.push_back(vertex_desc);
//    tfm::printfln("blah");
    return;
  }
};


static auto current_order = int32_t{0};
static auto unnumbered_vertices = std::queue<bb_vertex_desc_t>{};
static auto numbering_from_vertex (bb_vertex_desc_t current_vertex) -> void
{
  std::get<BB_ORDER>(internal_bb_graph[current_vertex]) = current_order;
  ++current_order;

  auto first_out_edge_iter = bb_graph_t::out_edge_iterator{};
  auto last_out_edge_iter = bb_graph_t::out_edge_iterator{};
  std::tie(first_out_edge_iter, last_out_edge_iter) = boost::out_edges(current_vertex, internal_bb_graph);

  auto next_unnumbered_vertices = std::vector<bb_vertex_desc_t>{};
  for (auto out_edge_iter = first_out_edge_iter; out_edge_iter != last_out_edge_iter; ++out_edge_iter) {
    auto next_vertex = boost::target(*out_edge_iter, internal_bb_graph);
    if (std::get<BB_ORDER>(internal_bb_graph[next_vertex]) == -1) {
      next_unnumbered_vertices.push_back(next_vertex);
    }
  }

  // deciding order of traversing
  std::stable_sort(std::begin(next_unnumbered_vertices), std::end(next_unnumbered_vertices), [](bb_vertex_desc_t va, bb_vertex_desc_t vb)
  {
    return (std::get<BB_ADDRESSES>(internal_bb_graph[va]).front() < std::get<BB_ADDRESSES>(internal_bb_graph[vb]).front());
  });

  for (auto next_vertex_desc : next_unnumbered_vertices) {
//    numbering_from_vertex(next_vertex_desc);
    unnumbered_vertices.push(next_vertex_desc);
  }

  return;
}


static auto numbering_bb_graph () -> void
{
  auto first_bb_iter = bb_vertex_iter_t{};
  auto last_bb_iter  = bb_vertex_iter_t{};
  std::tie(first_bb_iter, last_bb_iter) = boost::vertices(internal_bb_graph);

//  auto bb_iter = std::find_if(first_bb_iter, last_bb_iter, [](bb_vertex_desc_t bb_vertex_desc) {
//    return (boost::in_degree(bb_vertex_desc, internal_bb_graph) == 0);
//  });
  auto bb_iter = std::find_if(first_bb_iter, last_bb_iter, is_first_bb);
  assert(bb_iter != last_bb_iter);

  // deterministic BFS traversing
  auto first_bb_vertex_desc = *bb_iter;
  unnumbered_vertices.push(first_bb_vertex_desc);
  while (!unnumbered_vertices.empty()) {
    auto considered_vertex = unnumbered_vertices.front();
    unnumbered_vertices.pop();
    numbering_from_vertex(considered_vertex);
  }
//  numbering_from_vertex(first_bb_vertex_desc);

//  auto begin_bb_iter = bb_vertex_iter_t{};
//  auto end_bb_iter = bb_vertex_iter_t{};

//  auto start_bb_desc = bb_vertex_desc_t{};

//  assert(boost::num_vertices(internal_bb_graph) > 0);

//  std::tie(begin_bb_iter, end_bb_iter) = boost::vertices(internal_bb_graph);

//  auto visitor = node_numbering_visitor();
//  linear_visited_bbs.clear();
//  boost::breadth_first_search(internal_bb_graph, start_bb_desc, boost::visitor(visitor));

//  auto linear_order = 0;
//  tfm::printfln("size of graph %d", linear_visited_bbs.size());
//  for (auto& bb : linear_visited_bbs) {
//    std::get<BB_ORDER>(internal_bb_graph[bb]) = linear_order;
//    ++linear_order;
//  }

  return;
}


static auto construct_bb_graph () -> void
{
  auto first_vertex_iter = tr_vertex_iter_t();
  auto last_vertex_iter = tr_vertex_iter_t();

  std::tie(first_vertex_iter, last_vertex_iter) = boost::vertices(internal_graph);
  for (auto vertex_iter = first_vertex_iter; vertex_iter != last_vertex_iter; ++vertex_iter) {
    boost::add_vertex(bb_vertex_t(-1, tr_vertices_t{internal_graph[*vertex_iter]}), internal_bb_graph);
  }

  auto first_bb_vertex_iter = bb_vertex_iter_t();
  auto last_bb_vertex_iter = bb_vertex_iter_t();

  std::tie(first_bb_vertex_iter, last_bb_vertex_iter) = boost::vertices(internal_bb_graph);

  auto get_tr_vertex_desc = [](tr_vertex_t addr)  -> tr_vertex_desc_t
  {
    auto first_vertex_iter = tr_vertex_iter_t();
    auto last_vertex_iter = tr_vertex_iter_t();

    std::tie(first_vertex_iter, last_vertex_iter) = boost::vertices(internal_graph);
    for (auto vertex_iter = first_vertex_iter; vertex_iter != last_vertex_iter; ++vertex_iter) {
      if (internal_graph[*vertex_iter] == addr) return *vertex_iter;
    }
    return tr_graph_t::null_vertex();
  };

  for (auto src_bb_vertex_iter = first_bb_vertex_iter; src_bb_vertex_iter != last_bb_vertex_iter; ++src_bb_vertex_iter)
    for (auto dst_bb_vertex_iter = first_bb_vertex_iter; dst_bb_vertex_iter != last_bb_vertex_iter; ++dst_bb_vertex_iter) {

      auto src_tr_vertex_desc = get_tr_vertex_desc(std::get<BB_ADDRESSES>(internal_bb_graph[*src_bb_vertex_iter]).front());
      auto dst_tr_vertex_desc = get_tr_vertex_desc(std::get<BB_ADDRESSES>(internal_bb_graph[*dst_bb_vertex_iter]).front());

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

  numbering_bb_graph();

  return;
}


auto construct_bb_trace () -> std::vector<bb_vertex_desc_t>
{
  auto bb_trace = std::vector<bb_vertex_desc_t>{};

  auto ins_iter = std::begin(trace);
  auto last_ins_iter = std::end(trace);

  auto first_bb_iter = bb_vertex_iter_t{};
  auto last_bb_iter = bb_vertex_iter_t{};
  std::tie(first_bb_iter, last_bb_iter) = boost::vertices(internal_bb_graph);

//  auto bb_iter = std::find_if(first_bb_iter, last_bb_iter, [](bb_vertex_desc_t bb_vertex_desc) {
//    return (boost::in_degree(bb_vertex_desc, internal_bb_graph) == 0);
//  });
  auto bb_iter = std::find_if(first_bb_iter, last_bb_iter, is_first_bb);
  assert(bb_iter != last_bb_iter);

  auto bb_vertex_desc = *bb_iter;

  while (ins_iter != last_ins_iter) {
    for (auto & ins_addr : std::get<BB_ADDRESSES>(internal_bb_graph[bb_vertex_desc])) {
      assert(ins_addr == std::get<INS_ADDRESS>(*ins_iter));
      ++ins_iter;
    }

    bb_trace.push_back(bb_vertex_desc);

    if (ins_iter != last_ins_iter) {
      auto first_out_edge_iter = bb_graph_t::out_edge_iterator{};
      auto last_out_edge_iter = bb_graph_t::out_edge_iterator{};
      std::tie(first_out_edge_iter, last_out_edge_iter) = boost::out_edges(bb_vertex_desc, internal_bb_graph);

      auto next_out_edge_iter = std::find_if(first_out_edge_iter, last_out_edge_iter, [&](bb_edge_desc_t bb_edge_desc) {
        auto target_vertex_desc = boost::target(bb_edge_desc, internal_bb_graph);
        return (std::get<INS_ADDRESS>(*ins_iter) == std::get<BB_ADDRESSES>(internal_bb_graph[target_vertex_desc]).front());
      });

      assert(next_out_edge_iter != last_out_edge_iter);
      bb_vertex_desc = boost::target(*next_out_edge_iter, internal_bb_graph);
    }
  }

  return bb_trace;
}


using bb_vertex_descs_t = std::vector<bb_vertex_desc_t>;
auto construct_bbs_trace (const bb_vertex_descs_t& bb_trace) -> std::vector<bb_vertex_descs_t>
{
  auto next_bbs = [&](const bb_vertex_descs_t& input_bb_trace) -> std::pair<bb_vertex_descs_t, bb_vertex_descs_t> {
    assert((input_bb_trace[0] == bb_trace[0]) || (input_bb_trace[0] == bb_trace[1]));

    auto next_trace_iter = std::find_if(std::begin(input_bb_trace) + 1, std::end(input_bb_trace),
                                        [&](bb_vertex_desc_t bb_vertex_desc)
    {
      return (bb_vertex_desc == bb_trace[0]) || (bb_vertex_desc == bb_trace[1]);
    });

    return std::make_pair(bb_vertex_descs_t(std::begin(input_bb_trace), next_trace_iter),
                          bb_vertex_descs_t(next_trace_iter, std::end(input_bb_trace)));
  };

  if (bb_trace.size() >= 2) {
    auto remained_trace = bb_trace;
    auto bbs_trace = std::vector<bb_vertex_descs_t>{};

    while (!remained_trace.empty()) {
      auto next_bbs_pair = next_bbs(remained_trace);
      bbs_trace.push_back(std::get<0>(next_bbs_pair));
      remained_trace = std::get<1>(next_bbs_pair);
    }

    return bbs_trace;
  }
  else return std::vector<bb_vertex_descs_t>{bb_trace};
}

auto construct_bbs_graph (const std::vector<bb_vertex_descs_t> bbs_trace) -> void
{
  auto compute_bbs_vertex_value = [](bb_vertex_descs_t bb_vertex_descs) -> bbs_vertex_t {
    auto bbs_vertex_value = bbs_vertex_t{};
    for (const auto& vertex_desc : bb_vertex_descs) {
      bbs_vertex_value.push_back(std::get<BB_ORDER>(internal_bb_graph[vertex_desc]));
    }
    return bbs_vertex_value;
  };

  auto find_vertex = [&](const bb_vertex_descs_t&  bb_orders) -> bbs_vertex_iter_t {
    auto first_vertex_iter = bbs_vertex_iter_t();
    auto last_vertex_iter = bbs_vertex_iter_t();
    std::tie(first_vertex_iter, last_vertex_iter) = boost::vertices(internal_bbs_graph);

    auto bbs_vertex_value = compute_bbs_vertex_value(bb_orders);
    auto found_vertex_iter = std::find_if(first_vertex_iter, last_vertex_iter, [&](bbs_vertex_desc_t vertex_desc)
    {
      auto mismatched_pair = std::mismatch(std::begin(internal_bbs_graph[vertex_desc]), std::end(internal_bbs_graph[vertex_desc]),
                                           std::begin(bbs_vertex_value)/*, std::end(bbs_vertex_value)*/);

      return (std::get<0>(mismatched_pair) == std::end(internal_bbs_graph[vertex_desc])) &&
             (std::get<1>(mismatched_pair) == std::end(bbs_vertex_value));
    });

    return found_vertex_iter;
  };


  auto prev_vertex_desc = bbs_graph_t::null_vertex();
  for (const auto& bbs : bbs_trace) {
    auto curr_vertex_iter = find_vertex(bbs);
    auto curr_vertex_desc = bbs_vertex_desc_t();

    if (curr_vertex_iter == std::get<1>(boost::vertices(internal_bbs_graph))) {
      curr_vertex_desc = boost::add_vertex(compute_bbs_vertex_value(bbs), internal_bbs_graph);
    }
    else curr_vertex_desc = *curr_vertex_iter;

    if (prev_vertex_desc != bbs_graph_t::null_vertex()) {
      if (!std::get<1>(boost::edge(prev_vertex_desc, curr_vertex_desc, internal_bbs_graph))) {
        boost::add_edge(prev_vertex_desc, curr_vertex_desc, internal_bbs_graph);
      }
    }

    prev_vertex_desc = curr_vertex_desc;
  }

  return;
}


static auto is_loopback_bbss (bbss_vertex_desc_t vertex_desc) -> bool
{
  auto first_out_edge_iter = bbss_graph_t::out_edge_iterator();
  auto last_out_edge_iter = bbss_graph_t::out_edge_iterator();

  std::tie(first_out_edge_iter, last_out_edge_iter) = boost::out_edges(vertex_desc, internal_bbss_graph);

  return std::any_of(first_out_edge_iter, last_out_edge_iter, [&vertex_desc](bbss_edge_desc_t edge_desc)
  {
    return (boost::target(edge_desc, internal_bbss_graph) == vertex_desc);
  });
}

static auto is_first_bbss (bbss_vertex_desc_t vertex_desc) -> bool
{
  return ((internal_bbss_graph[vertex_desc].front().size() == 1) &&
          (internal_bbss_graph[vertex_desc].front().front() == 0));
}


static auto find_pivot_bbss_vertex () -> bbss_vertex_desc_t
{
  auto first_vertex_iter = bbss_vertex_iter_t();
  auto last_vertex_iter = bbss_vertex_iter_t();
  std::tie(first_vertex_iter, last_vertex_iter) = boost::vertices(internal_bbss_graph);

  auto found_vertex_iter = std::find_if(first_vertex_iter, last_vertex_iter, [](bbss_vertex_desc_t current_vertex)
  {
    if (!is_loopback_bbss(current_vertex)) {
      auto in_degree = boost::in_degree(current_vertex, internal_bbss_graph);
      auto out_degree = boost::out_degree(current_vertex, internal_bbss_graph);

      if (out_degree == 1) {
        auto out_edge_iter = bbss_graph_t::out_edge_iterator();
        std::tie(out_edge_iter, std::ignore) = boost::out_edges(current_vertex, internal_bbss_graph);

        auto next_vertex = boost::target(*out_edge_iter, internal_bbss_graph);

        if ((boost::in_degree(next_vertex, internal_bbss_graph) == 1) && !is_loopback_bbss(next_vertex) && !is_first_bbss(next_vertex)) {
          if ((in_degree == 0) || (in_degree >= 2)) {
            return true;
          }
          else {
            auto in_edge_iter = bbss_graph_t::in_edge_iterator();
            std::tie(in_edge_iter, std::ignore) = boost::in_edges(current_vertex, internal_bbss_graph);

            auto prev_vertex = boost::source(*in_edge_iter, internal_bbss_graph);
            if (boost::out_degree(prev_vertex, internal_bbss_graph) >= 2) {
              return (is_loopback_bbss(prev_vertex) || (boost::out_degree(prev_vertex, internal_bbss_graph) >= 2));
            }
            else return is_first_bbss(current_vertex);
          }
        }
        else return false;
      }
      else return false;
    }

    return false;
  });

  if (found_vertex_iter != last_vertex_iter) return *found_vertex_iter;
  else return bbss_graph_t::null_vertex();
}


static auto compress_bbss_graph_from_pivot_vertex (bbss_vertex_desc_t pivot_vertex) -> void
{
  auto out_edge_iter = bbss_graph_t::out_edge_iterator();
  std::tie(out_edge_iter, std::ignore) = boost::out_edges(pivot_vertex, internal_bbss_graph);

  auto next_vertex = boost::target(*out_edge_iter, internal_bbss_graph);

  boost::remove_out_edge_if(pivot_vertex, [](bbss_edge_desc_t edge_desc) { return true; }, internal_bbss_graph);
  internal_bbss_graph[pivot_vertex].insert(std::end(internal_bbss_graph[pivot_vertex]),
                                           std::begin(internal_bbss_graph[next_vertex]),
                                           std::end(internal_bbss_graph[next_vertex]));

  auto last_out_edge_iter = bbss_graph_t::out_edge_iterator();
  std::tie(out_edge_iter, last_out_edge_iter) = boost::out_edges(next_vertex, internal_bbss_graph);

  for (auto edge_iter = out_edge_iter; edge_iter != last_out_edge_iter; ++edge_iter) {
    auto next_next_vertex = boost::target(*edge_iter, internal_bbss_graph);
    boost::add_edge(pivot_vertex, next_next_vertex, internal_bbss_graph);
  }
  boost::remove_out_edge_if(next_vertex, [](bbss_edge_desc_t edge_desc) { return true; }, internal_bbss_graph);
  boost::remove_vertex(next_vertex, internal_bbss_graph);
}


static auto construct_bbss_graph () -> void
{
  auto first_vertex_iter = bbs_vertex_iter_t();
  auto last_vertex_iter = bbs_vertex_iter_t();

  std::tie(first_vertex_iter, last_vertex_iter) = boost::vertices(internal_bbs_graph);
  for (auto vertex_iter = first_vertex_iter; vertex_iter != last_vertex_iter; ++vertex_iter) {
    boost::add_vertex(bbss_vertex_t{internal_bbs_graph[*vertex_iter]}, internal_bbss_graph);
  }

  auto corresponding_bbs_vertex = [](bbss_vertex_desc_t bbss_vertex_desc) -> bbs_vertex_desc_t {
    auto first_bbs_vertex_iter = bbs_vertex_iter_t();
    auto last_bbs_vertex_iter = bbs_vertex_iter_t();

    std::tie(first_bbs_vertex_iter, last_bbs_vertex_iter) = boost::vertices(internal_bbs_graph);
    for (auto bbs_vertex_iter = first_bbs_vertex_iter; bbs_vertex_iter != last_bbs_vertex_iter; ++bbs_vertex_iter) {
      auto mismatched_pair = std::mismatch(std::begin(internal_bbs_graph[*bbs_vertex_iter]),
          std::end(internal_bbs_graph[*bbs_vertex_iter]), std::begin(internal_bbss_graph[bbss_vertex_desc].front()));
      if ((std::get<0>(mismatched_pair) == std::end(internal_bbs_graph[*bbs_vertex_iter])) &&
          (std::get<1>(mismatched_pair) == std::end(internal_bbss_graph[bbss_vertex_desc].front()))) {
        return *bbs_vertex_iter;
      }
    }
    return bbs_graph_t::null_vertex();
  };

  auto first_bbss_vertex_iter = bbss_vertex_iter_t();
  auto last_bbss_vertex_iter = bbss_vertex_iter_t();

  std::tie(first_bbss_vertex_iter, last_bbss_vertex_iter) = boost::vertices(internal_bbss_graph);
  for (auto src_bbss_vertex_iter = first_bbss_vertex_iter; src_bbss_vertex_iter != last_bbss_vertex_iter; ++src_bbss_vertex_iter) {
    for (auto dst_bbss_vertex_iter = first_bbss_vertex_iter; dst_bbss_vertex_iter != last_bbss_vertex_iter; ++dst_bbss_vertex_iter) {
      auto src_bbs_vertex_desc = corresponding_bbs_vertex(*src_bbss_vertex_iter);
      auto dst_bbs_vertex_desc = corresponding_bbs_vertex(*dst_bbss_vertex_iter);

      if (std::get<1>(boost::edge(src_bbs_vertex_desc, dst_bbs_vertex_desc, internal_bbs_graph)) &&
          !std::get<1>(boost::edge(*src_bbss_vertex_iter, *dst_bbss_vertex_iter, internal_bbss_graph))) {
        boost::add_edge(*src_bbss_vertex_iter, *dst_bbss_vertex_iter, internal_bbss_graph);
      }
    }
  }

  do {
    auto pivot_vertex_desc = find_pivot_bbss_vertex();
    if (pivot_vertex_desc != bbss_graph_t::null_vertex()) {
      compress_bbss_graph_from_pivot_vertex(pivot_vertex_desc);
    }
    else break;
  }
  while (true);

  return;
}

// come back later!!!
//static auto serialize_bb_graph (const std::string& filename) -> void
//{
//  std::ofstream output_file(filename.c_str(), std::ofstream::out | std::ofstream::binary);
//  if (output_file.is_open()) {
//    boost::archive::binary_oarchive boa(output_file);
//    boa << internal_bb_graph;
//    output_file.close();
//  }
//  return;
//}


/*====================================================================================================================*/
/*                                                     exported functions                                             */
/*====================================================================================================================*/

auto cap_save_trace_to_dot_file (const std::string& filename) noexcept -> void
{
  auto write_vertex = [](std::ostream& label, tr_vertex_desc_t vertex_desc) -> void {
    tfm::format(label, "[label=\"%s:%s\"]", normalize_hex_string(StringFromAddrint(internal_graph[vertex_desc])),
                cached_ins_at_addr[internal_graph[vertex_desc]]->disassemble);
    return;
  };

  auto write_edge = [](std::ostream& label, tr_edge_desc_t edge_desc) -> void {
    tfm::format(label, "[label=\"\"]");
    return;
  };

  if (trace.size() > 0) {
    construct_graph_from_trace();

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
  }
  else tfm::printfln("trace is empty, graph contruction is omitted");

  return;
}


auto cap_save_basic_block_trace_to_dot_file (const std::string& filename) noexcept -> void
{
  auto write_vertex = [](std::ostream& label, bb_vertex_desc_t vertex_desc) -> void {

    if (is_first_bb(vertex_desc)) {
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

    tfm::format(label, "%d\n", std::get<BB_ORDER>(internal_bb_graph[vertex_desc]));
    for (const auto& addr : std::get<BB_ADDRESSES>(internal_bb_graph[vertex_desc])) {
      /*if (std::addressof(addr) == std::addressof(internal_bb_graph[vertex_desc].back())) {
        tfm::format(label, "%-12s %-s", StringFromAddrint(addr), cached_ins_at_addr[addr]->disassemble);
      }
      else*/
      tfm::format(label, "%-12s %-s\\l", normalize_hex_string(StringFromAddrint(addr)), cached_ins_at_addr[addr]->disassemble);
    }
//    tfm::format(label, "\",fontname=Courier,fontsize=10.0]");
    tfm::format(label, "\",fontname=\"Inconsolata\",fontsize=10.0]");

    return;
  };

  auto write_edge = [](std::ostream& label, bb_edge_desc_t edge_desc) -> void {
    tfm::format(label, "[label=\"\"]");
    return;
  };

  if (boost::num_vertices(internal_graph) > 0) {
    construct_bb_graph();

    ofstream output_file(filename.c_str(), std::ofstream::out | std::ofstream::trunc);
    if (output_file.is_open()) {
      boost::write_graphviz(output_file, internal_bb_graph,
                            std::bind(write_vertex, std::placeholders::_1, std::placeholders::_2),
                            std::bind(write_edge, std::placeholders::_1, std::placeholders::_2));
      output_file.close();
    }
    else {
      tfm::printfln("cannot save basic block graph to file %s", filename);
    }
  }
  else tfm::printfln("graph is empty, constructing basic block graph is omitted");

  return;
}


auto cap_save_basic_block_trace_to_file (const std::string& filename) noexcept -> void
{
  auto is_first_bbs = [](bbs_vertex_desc_t vertex_desc) -> bool {
    auto vertex_value = internal_bbs_graph[vertex_desc];
    return (vertex_value.size() == 1) && (vertex_value[0] == 0);
  };

  auto write_vertex = [&](std::ostream& label, bbs_vertex_desc_t vertex_desc) -> void
  {
    if (is_first_bbs(vertex_desc)) {
      tfm::format(label, "[shape=box,style=\"filled,rounded\",fillcolor=cornflowerblue,label=\"");
    }
    else if (boost::out_degree(vertex_desc, internal_bbs_graph) == 0) {
      tfm::format(label, "[shape=box,style=\"filled,rounded\",fillcolor=gainsboro,label=\"");
    }
    else if (boost::in_degree(vertex_desc, internal_bbs_graph) > 2) {
      tfm::format(label, "[shape=box,style=\"filled,rounded\",fillcolor=darkorchid1,label=\"");
    }
    else if (boost::out_degree(vertex_desc, internal_bbs_graph) > 2) {
      tfm::format(label, "[shape=box,style=\"filled,rounded\",fillcolor=darkgoldenrod1,label=\"");
    }
    else tfm::format(label, "[shape=box,style=rounded,label=\"");

    auto count = uint32_t{0};
    for (const auto& bb_order : internal_bbs_graph[vertex_desc]) {
      tfm::format(label, "%d ", bb_order);
      count++;
      if (count % 7 == 0) tfm::format(label, "\\l");
    }

    tfm::format(label, "\",fontname=\"Inconsolata\",fontsize=10.0]");
  };

  auto write_edge = [](std::ostream& label, bbs_edge_desc_t edge_desc) -> void {
    tfm::format(label, "[label=\"\"]");
    return;
  };

  auto is_first_bbss = [](bbss_vertex_desc_t vertex_desc) -> bool
  {
    return ((internal_bbss_graph[vertex_desc].size() == 1) && (internal_bbss_graph[vertex_desc].front().front() == 0));
  };

  auto write_bbss_vertex = [&](std::ostream& label, bbss_vertex_desc_t vertex_desc) -> void
  {
    if (is_first_bbss(vertex_desc)) {
      tfm::format(label, "[shape=box,style=\"filled,rounded\",fillcolor=cornflowerblue,label=\"");
    }
    else if (boost::out_degree(vertex_desc, internal_bbss_graph) == 0) {
      tfm::format(label, "[shape=box,style=\"filled,rounded\",fillcolor=gainsboro,label=\"");
    }
    else if (boost::in_degree(vertex_desc, internal_bbss_graph) > 2) {
      tfm::format(label, "[shape=box,style=\"filled,rounded\",fillcolor=darkorchid1,label=\"");
    }
    else if (boost::out_degree(vertex_desc, internal_bbss_graph) > 2) {
      tfm::format(label, "[shape=box,style=\"filled,rounded\",fillcolor=darkgoldenrod1,label=\"");
    }
    else tfm::format(label, "[shape=box,style=rounded,label=\"");

    for (const auto& bbs_value : internal_bbss_graph[vertex_desc]) {
//      auto count = uint32_t{0};
      for (const auto& bb_order : bbs_value) {
        tfm::format(label, "%d ", bb_order);
//        count++; if (count % 7 == 0) tfm::format(label, "\\l");
      }
      tfm::format(label, "\\l");
    }

    tfm::format(label, "\",fontname=\"Inconsolata\",fontsize=10.0]");
  };

  auto write_bbss_edge = [](std::ostream& label, bbss_edge_desc_t edge_desc) -> void
  {
    tfm::format(label, "[label=\"\"]");;
  };

  ofstream output_file(filename.c_str(), std::ofstream::out | std::ofstream::trunc);
  if (output_file.is_open()) {

    if (boost::num_vertices(internal_bb_graph) > 0) {
      auto bb_trace = construct_bb_trace();

//      for (auto & bb_vertex_desc : bb_trace) {
//        tfm::format(output_file, "%d ", std::get<BB_ORDER>(internal_bb_graph[bb_vertex_desc]));
//      }

      auto bbs_trace = construct_bbs_trace(bb_trace);

      for (auto & bb_trace : bbs_trace) {
        for (auto & bb_vertex_desc : bb_trace) {
          tfm::format(output_file, "%d ", std::get<BB_ORDER>(internal_bb_graph[bb_vertex_desc]));
        }
        tfm::format(output_file, "\n");
      }

//      construct_bbs_graph(bbs_trace);

//      boost::write_graphviz(output_file, internal_bbs_graph,
//                            std::bind(write_vertex, std::placeholders::_1, std::placeholders::_2),
//                            std::bind(write_edge, std::placeholders::_1, std::placeholders::_2));

//      construct_bbss_graph();

//      boost::write_graphviz(output_file, internal_bbss_graph,
//                            std::bind(write_bbss_vertex, std::placeholders::_1, std::placeholders::_2),
//                            std::bind(write_bbss_edge, std::placeholders::_1, std::placeholders::_2));
    }
    else tfm::printfln("basic block graph is empty, constructing basic block trace is omitted");
//    tfm::format(output_file, "\n");

    output_file.close();
  }
  else {
    tfm::printfln("cannot save bb-trace to file %s", filename);
  }

  return;
}
