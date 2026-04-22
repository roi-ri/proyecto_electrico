cmake_minimum_required(VERSION 3.20)
project(batView VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

add_executable(batView
    src/main.cpp

    src/core/models/SessionData.cpp

    src/core/protocol/ProtocolEncoder.cpp
    src/core/protocol/ProtocolParser.cpp

    src/core/services/ConnectionService.cpp

    src/infrastructure/logging/FileLogger.cpp

    src/ui/frames/MainFrame.cpp
)

target_include_directories(batView PRIVATE src)

target_link_libraries(batView PRIVATE ${wxWidgets_LIBRARIES})
