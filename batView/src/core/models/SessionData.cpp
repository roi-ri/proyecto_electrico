#include "core/models/SessionData.h"

namespace batview::core::models {

void SessionData::AddMeasurement(const Measurement& measurement) {
    std::lock_guard<std::mutex> lock(mutex_);
    measurements_.push_back(measurement);
}

std::vector<Measurement> SessionData::GetAllMeasurements() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return measurements_;
}

void SessionData::Clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    measurements_.clear();
}

std::size_t SessionData::Size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return measurements_.size();
}

} // namespace batview::core::models
