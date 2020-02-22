#ifndef FUZILLY_TRACEREADER_H
#define FUZILLY_TRACEREADER_H

#include <fstream>
#include <optional>

#include "../../../../common/fzl.h"

namespace fzl {

namespace Retracer {

class TraceReader {
private:
  std::ifstream TraceStream;

public:
  explicit TraceReader(const char *FileName);
  ~TraceReader() = default;

  std::optional<fzl::TraceEvent> getNext();
  bool isOk() { return bool(TraceStream); }
};

} // namespace Retracer

} // namespace fzl


#endif //FUZILLY_TRACEREADER_H
