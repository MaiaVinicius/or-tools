// Copyright 2010-2018 Google LLC
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef OR_TOOLS_SAT_PRESOLVE_UTIL_H_
#define OR_TOOLS_SAT_PRESOLVE_UTIL_H_

#include <algorithm>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "ortools/base/int_type.h"
#include "ortools/base/int_type_indexed_vector.h"
#include "ortools/base/integral_types.h"
#include "ortools/base/logging.h"
#include "ortools/util/bitset.h"
#include "ortools/util/sorted_interval_list.h"

namespace operations_research {
namespace sat {

// If for each literal of a clause, we can infer a domain on an integer
// variable, then we know that this variable domain is included in the union of
// such infered domains.
//
// This allows to propagate "element" like constraints encoded as enforced
// linear relations, and other more general reasoning.
//
// TODO(user): Also use these "deductions" in the solver directly. This is done
// in good MIP solvers, and we should exploit them more.
class DomainDeductions {
 public:
  // Adds the fact that enforcement => var \in domain.
  void AddDeduction(int literal_ref, int var, Domain domain);

  // Returns list of (var, domain) that were deduced because:
  //   1/ We have a domain deduction for var and all literal from the clause
  //   2/ So we can take the union of all the deduced domains.
  //
  // TODO(user): We could probably be even more efficient. We could also
  // compute exactly what clauses need to be "waked up" as new deductions are
  // added.
  std::vector<std::pair<int, Domain>> ProcessClause(
      absl::Span<const int> clause);

  // Optimization. Any following ProcessClause() will be fast if no more
  // deduction touching that clause are added.
  void MarkProcessingAsDoneForNow() {
    something_changed_.ClearAndResize(something_changed_.size());
  }

  // Returns the total number of "deductions" stored by this class.
  int NumDeductions() const { return deductions_.size(); }

 private:
  DEFINE_INT_TYPE(Index, int);
  Index IndexFromLiteral(int ref) {
    return Index(ref >= 0 ? 2 * ref : -2 * ref - 1);
  }

  std::vector<int> tmp_num_occurences_;

  SparseBitset<Index> something_changed_;
  gtl::ITIVector<Index, std::vector<int>> enforcement_to_vars_;
  absl::flat_hash_map<std::pair<Index, int>, Domain> deductions_;
};

}  // namespace sat
}  // namespace operations_research

#endif  // OR_TOOLS_SAT_PRESOLVE_UTIL_H_