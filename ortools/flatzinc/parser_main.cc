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

// This binary reads an input file in the flatzinc format (see
// http://www.minizinc.org/), parses it, and spits out the model it
// has built.

#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "ortools/base/commandlineflags.h"
#include "ortools/base/timer.h"
#include "ortools/flatzinc/logging.h"
#include "ortools/flatzinc/model.h"
#include "ortools/flatzinc/parser.h"
#include "ortools/flatzinc/presolve.h"

ABSL_FLAG(std::string, file, "", "Input file in the flatzinc format.");
ABSL_FLAG(bool, print, false, "Print model.");
ABSL_FLAG(bool, presolve, false, "Presolve loaded file.");
ABSL_FLAG(bool, statistics, false, "Print model statistics");

namespace operations_research {
namespace fz {
void ParseFile(const std::string& filename, bool presolve) {
  WallTimer timer;
  timer.Start();

  FZLOG << "Loading " << filename << std::endl;

  std::string problem_name = filename;
  // Remove the .fzn extension.
  CHECK(absl::EndsWith(problem_name, ".fzn"));
  problem_name.resize(problem_name.size() - 4);
  // Remove the leading path if present.
  const size_t found = problem_name.find_last_of("/\\");
  if (found != std::string::npos) {
    problem_name = problem_name.substr(found + 1);
  }
  FZLOG << "  - parsed in " << timer.GetInMs() << " ms" << std::endl;

  Model model(problem_name);
  CHECK(ParseFlatzincFile(filename, &model));
  if (presolve) {
    FZLOG << "Presolve model" << std::endl;
    timer.Reset();
    timer.Start();
    Presolver presolve;
    presolve.Run(&model);
    FZLOG << "  - done in " << timer.GetInMs() << " ms" << std::endl;
  }
  if (absl::GetFlag(FLAGS_statistics)) {
    ModelStatistics stats(model);
    stats.BuildStatistics();
    stats.PrintStatistics();
  }
  if (absl::GetFlag(FLAGS_print)) {
    FZLOG << model.DebugString() << std::endl;
  }
}
}  // namespace fz
}  // namespace operations_research

int main(int argc, char** argv) {
  const char kUsage[] =
      "Parses a flatzinc .fzn file, optionally presolve it, and prints it in "
      "human-readable format";
  absl::SetFlag(&FLAGS_log_prefix, false);
  absl::SetFlag(&FLAGS_logtostderr, true);
  absl::SetProgramUsageMessage(kUsage);
  absl::ParseCommandLine(argc, argv);
  google::InitGoogleLogging(argv[0]);
  operations_research::fz::ParseFile(absl::GetFlag(FLAGS_file),
                                     absl::GetFlag(FLAGS_presolve));
  return 0;
}
