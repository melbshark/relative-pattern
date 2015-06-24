#include "tinyformat.h"

#include <vector>
#include <cstdint>
#include <algorithm>

typedef uint32_t virtual_ins_t;
typedef std::vector<virtual_ins_t> virtual_bb_t;
typedef std::vector<virtual_bb_t> virtual_bbs_t;
typedef std::vector<virtual_ins_t> virtual_trace_t;
typedef std::vector<virtual_trace_t> virtual_traces_t;

auto get_prefix_bb (const virtual_traces_t& traces) -> virtual_bb_t
{
  auto root_ins = traces.front().front();
  assert(std::all_of(std::begin(traces), std::end(traces), [&](virtual_trace_t trace)
  { return (trace.front() == root_ins); }));

  auto min_trace_iter = std::min_element(
        std::begin(traces), std::end(traces), [](const virtual_trace_t& trace_a, const virtual_trace_t& trace_b
        )
  {
    return (trace_a.size() < trace_b.size());
  });

  auto is_prefix_of = [](const virtual_trace_t& trace_a, const virtual_trace_t& trace_b) -> bool
  {
    auto res = std::mismatch(std::begin(trace_a), std::end(trace_a), std::begin(trace_b));
    return (res.first == std::end(trace_a));
  };

  auto prefix = virtual_trace_t{};
  for (auto ins : *min_trace_iter) {
    prefix.push_back(ins);
    if (!std::all_of(std::begin(traces), std::end(traces), [&](virtual_trace_t trace)
    { return is_prefix_of(prefix, trace); })) {
      prefix.pop_back();
    }
  }

  return prefix;
}


auto seperate_by_root (const virtual_traces_t& traces) -> std::vector<virtual_traces_t>
{
  auto remained_traces = traces;
  auto groups = std::vector<virtual_traces_t>{};

  while (!remained_traces.empty()) {
    auto root_ins = remained_traces.front().front();
    auto group = virtual_traces_t{};

    for (const auto& trace : remained_traces) {
      if (trace.front() == root_ins) group.push_back(trace);
    }
    groups.push_back(group);

    for (const auto& trace : group) {
      auto trace_iter = std::find_if(
            std::begin(remained_traces), std::end(remained_traces), [&](const virtual_trace_t& in
            )
      {
        return (in.size() == trace.size()) && std::equal(std::begin(trace), std::end(trace), std::begin(in));
      });

      remained_traces.erase(trace_iter);
    }
  }

  return groups;
}


auto get_suffix_traces (const virtual_trace_t& prefix,
                        const virtual_traces_t& traces) -> virtual_traces_t
{
  auto suffix_traces = virtual_traces_t{};

  for (const auto& trace : traces) {
    auto res = std::mismatch(std::begin(prefix), std::end(prefix), std::begin(trace));
    auto suffix = virtual_trace_t(res.second, std::end(trace));
    if (!suffix.empty()) {
      suffix_traces.push_back(suffix);
    }
  }

  return suffix_traces;
}
