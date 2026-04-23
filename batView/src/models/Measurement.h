#pragma once

#include <cstdint>

namespace batview::core::models {

struct Measurement {
    std::uint64_t timestampMs = 0;
    double voltage = 0.0;
    double current = 0.0;
    int state = -1;
    int completedCycles = 0;
};

} // namespace batview::core::models
