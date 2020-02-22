#include <cassert>

#include "TraceReader.h"

namespace fzl {

namespace Retracer {

TraceReader::TraceReader(const char *FileName) {
  TraceStream.open(FileName, std::ios::in | std::ios::binary);
}

std::optional<fzl::TraceEvent> TraceReader::getNext() {
  TraceEvent Event;

  // Read the structure tag
  if (!TraceStream.read(reinterpret_cast<char *>(&Event.Kind), sizeof(Event.Kind))) {
    return {};
  }

  // Read the fields manually
  switch (Event.Kind) {
  case TEK_BasicBlock: {
    TraceStream.read(
        reinterpret_cast<char *>(&Event.BasicBlock.Id),
        sizeof(Event.BasicBlock.Id)
    );
    break;
  }
  case TEK_MemoryLoad:
  case TEK_MemoryStore: {
    TraceStream.read(
        reinterpret_cast<char *>(&Event.MemoryAccess.Address),
        sizeof(Event.MemoryAccess.Address)
    );
    break;
  }
  default:assert(false && "Unknown event tag!");
  }

  // Check that we read everything OK
  if (!TraceStream) {
    return {};
  } else {
    return Event;
  }
}

} // namespace Retracer

} // namespace fzl

