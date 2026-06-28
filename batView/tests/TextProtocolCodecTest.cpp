#include <gtest/gtest.h>

#include <stdexcept>

#include "core/protocol/TextProtocolCodec.h"

namespace batview::core::protocol {

TEST(TextProtocolCodecTest, BatteryProfileAllowsZeroMinimumVoltage) {
    TextProtocolCodec codec;

    const auto command = codec.BuildBatteryProfileCommand({"LiIon_1S", 4.2, 0.0, 1.5});

    EXPECT_EQ(command, "#Battery,LiIon_1S,4.2,0,1.5");
}

TEST(TextProtocolCodecTest, BatteryProfileRejectsNegativeMinimumVoltage) {
    TextProtocolCodec codec;

    EXPECT_THROW(codec.BuildBatteryProfileCommand({"LiIon_1S", 4.2, -0.1, 1.5}), std::invalid_argument);
}

} // namespace batview::core::protocol
