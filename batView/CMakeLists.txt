cmake_minimum_required(VERSION 3.16)

project(batView VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BATVIEW_ENABLE_WX "Build wxWidgets UI layer" OFF)
option(BATVIEW_BUILD_TESTS "Build unit tests" OFF)
option(BATVIEW_BUILD_MACOS_ICON "Build macOS app icon bundle assets" OFF)
option(BATVIEW_ENABLE_EMBEDDED_PYTHON "Enable embedded Python plot/export runtime" OFF)
set(BATVIEW_EMBEDDED_PYTHON_ROOT "" CACHE PATH "Embedded Python root directory")
set(BATVIEW_DIST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dist" CACHE PATH "Directory where release packages will be written")

if(BATVIEW_ENABLE_EMBEDDED_PYTHON AND BATVIEW_EMBEDDED_PYTHON_ROOT)
    find_path(BATVIEW_EMBEDDED_PYTHON_INCLUDE
        NAMES Python.h
        HINTS
            "${BATVIEW_EMBEDDED_PYTHON_ROOT}/include"
            "${BATVIEW_EMBEDDED_PYTHON_ROOT}/include/python3.12"
            "${BATVIEW_EMBEDDED_PYTHON_ROOT}/include/python3.11"
        NO_DEFAULT_PATH
    )

    find_library(BATVIEW_EMBEDDED_PYTHON_LIBRARY
        NAMES python312 python311 python3.12 python3.11
        HINTS
            "${BATVIEW_EMBEDDED_PYTHON_ROOT}/libs"
            "${BATVIEW_EMBEDDED_PYTHON_ROOT}/lib"
        NO_DEFAULT_PATH
    )

    if(WIN32)
        set(BATVIEW_EMBEDDED_PYTHON_LIB "${BATVIEW_EMBEDDED_PYTHON_ROOT}/Lib")
        set(BATVIEW_EMBEDDED_PYTHON_SITE_PACKAGES "${BATVIEW_EMBEDDED_PYTHON_ROOT}/Lib/site-packages")
    else()
        file(GLOB BATVIEW_EMBEDDED_PYTHON_STDLIB_DIRS "${BATVIEW_EMBEDDED_PYTHON_ROOT}/lib/python3.*")
        list(LENGTH BATVIEW_EMBEDDED_PYTHON_STDLIB_DIRS BATVIEW_EMBEDDED_PYTHON_STDLIB_COUNT)
        if(BATVIEW_EMBEDDED_PYTHON_STDLIB_COUNT GREATER 0)
            list(GET BATVIEW_EMBEDDED_PYTHON_STDLIB_DIRS 0 BATVIEW_EMBEDDED_PYTHON_LIB)
            set(BATVIEW_EMBEDDED_PYTHON_SITE_PACKAGES "${BATVIEW_EMBEDDED_PYTHON_LIB}/site-packages")
        else()
            set(BATVIEW_EMBEDDED_PYTHON_LIB "")
            set(BATVIEW_EMBEDDED_PYTHON_SITE_PACKAGES "")
        endif()
    endif()
else()
    unset(BATVIEW_EMBEDDED_PYTHON_INCLUDE CACHE)
    unset(BATVIEW_EMBEDDED_PYTHON_LIBRARY CACHE)
    set(BATVIEW_EMBEDDED_PYTHON_INCLUDE "")
    set(BATVIEW_EMBEDDED_PYTHON_LIBRARY "")
    set(BATVIEW_EMBEDDED_PYTHON_LIB "")
    set(BATVIEW_EMBEDDED_PYTHON_SITE_PACKAGES "")
endif()

if(BATVIEW_ENABLE_EMBEDDED_PYTHON
   AND BATVIEW_EMBEDDED_PYTHON_ROOT
   AND EXISTS "${BATVIEW_EMBEDDED_PYTHON_INCLUDE}/Python.h"
   AND EXISTS "${BATVIEW_EMBEDDED_PYTHON_LIBRARY}")
    set(BATVIEW_EMBEDDED_PYTHON_ACTIVE ON)
else()
    set(BATVIEW_EMBEDDED_PYTHON_ACTIVE OFF)
endif()

set(BATVIEW_SERIAL_SOURCES
    src/infrastructure/serial/SerialPortStub.cpp
)

if(WIN32)
    list(APPEND BATVIEW_SERIAL_SOURCES src/infrastructure/serial/Win32SerialPort.cpp)
else()
    list(APPEND BATVIEW_SERIAL_SOURCES src/infrastructure/serial/PosixSerialPort.cpp)
endif()

set(BATVIEW_CORE_SOURCES
    src/app/AppController.cpp
    src/core/protocol/TextProtocolCodec.cpp
    src/core/protocol/TextMessageParser.cpp
    src/core/services/AcquisitionService.cpp
    src/core/services/ConnectionService.cpp
    src/core/services/ExportService.cpp
    src/core/services/PlotService.cpp
    src/core/services/ProtocolSessionService.cpp
    src/infrastructure/csv/CsvExporter.cpp
    src/infrastructure/logging/ConsoleLogger.cpp
    src/infrastructure/python/PythonPlotEngine.cpp
    ${BATVIEW_SERIAL_SOURCES}
    src/ui/viewmodels/MainViewModel.cpp
)

set(BATVIEW_APP_SOURCES
    src/main.cpp
    ${BATVIEW_CORE_SOURCES}
)

if(BATVIEW_ENABLE_WX)
    find_package(wxWidgets REQUIRED COMPONENTS base core adv)
    include(${wxWidgets_USE_FILE})
    list(APPEND BATVIEW_APP_SOURCES
        src/ui/BatViewApp.cpp
        src/ui/frames/MainFrame.cpp
        src/ui/dialogs/ExportDialog.cpp
        src/ui/panels/ConnectionPanel.cpp
        src/ui/panels/PlotPanel.cpp
    )

    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/assets)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/BatView.png
                   ${CMAKE_CURRENT_BINARY_DIR}/assets/BatView.png
                   COPYONLY)

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/batView.gif
                   ${CMAKE_CURRENT_BINARY_DIR}/assets/batView.gif
                   COPYONLY)

    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/assets/BatView.ico)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/BatView.ico
                       ${CMAKE_CURRENT_BINARY_DIR}/assets/BatView.ico
                       COPYONLY)
    endif()

    if(APPLE)
        set(BATVIEW_ICONSET_DIR ${CMAKE_CURRENT_BINARY_DIR}/BatView.iconset)
        set(BATVIEW_ICNS_FILE ${CMAKE_CURRENT_BINARY_DIR}/BatView.icns)
        find_program(BATVIEW_ICONUTIL_EXECUTABLE iconutil)

        if(BATVIEW_ICONUTIL_EXECUTABLE AND BATVIEW_BUILD_MACOS_ICON)
            add_custom_command(
                OUTPUT ${BATVIEW_ICNS_FILE}
                COMMAND ${CMAKE_COMMAND} -E rm -rf ${BATVIEW_ICONSET_DIR}
                COMMAND ${CMAKE_COMMAND} -E make_directory ${BATVIEW_ICONSET_DIR}
                COMMAND sips -z 16 16 ${CMAKE_CURRENT_SOURCE_DIR}/assets/BatView.png --out ${BATVIEW_ICONSET_DIR}/icon_16x16.png
                COMMAND sips -z 32 32 ${CMAKE_CURRENT_SOURCE_DIR}/assets/BatView.png --out ${BATVIEW_ICONSET_DIR}/icon_16x16@2x.png
                COMMAND sips -z 32 32 ${CMAKE_CURRENT_SOURCE_DIR}/assets/BatView.png --out ${BATVIEW_ICONSET_DIR}/icon_32x32.png
                COMMAND sips -z 64 64 ${CMAKE_CURRENT_SOURCE_DIR}/assets/BatView.png --out ${BATVIEW_ICONSET_DIR}/icon_32x32@2x.png
                COMMAND sips -z 128 128 ${CMAKE_CURRENT_SOURCE_DIR}/assets/BatView.png --out ${BATVIEW_ICONSET_DIR}/icon_128x128.png
                COMMAND sips -z 256 256 ${CMAKE_CURRENT_SOURCE_DIR}/assets/BatView.png --out ${BATVIEW_ICONSET_DIR}/icon_128x128@2x.png
                COMMAND sips -z 256 256 ${CMAKE_CURRENT_SOURCE_DIR}/assets/BatView.png --out ${BATVIEW_ICONSET_DIR}/icon_256x256.png
                COMMAND sips -z 512 512 ${CMAKE_CURRENT_SOURCE_DIR}/assets/BatView.png --out ${BATVIEW_ICONSET_DIR}/icon_256x256@2x.png
                COMMAND sips -z 512 512 ${CMAKE_CURRENT_SOURCE_DIR}/assets/BatView.png --out ${BATVIEW_ICONSET_DIR}/icon_512x512.png
                COMMAND sips -z 1024 1024 ${CMAKE_CURRENT_SOURCE_DIR}/assets/BatView.png --out ${BATVIEW_ICONSET_DIR}/icon_512x512@2x.png
                COMMAND ${BATVIEW_ICONUTIL_EXECUTABLE} -c icns ${BATVIEW_ICONSET_DIR} -o ${BATVIEW_ICNS_FILE}
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/assets/BatView.png
                VERBATIM
            )

            add_custom_target(batView_icon ALL DEPENDS ${BATVIEW_ICNS_FILE})
            list(APPEND BATVIEW_APP_SOURCES ${BATVIEW_ICNS_FILE})
        endif()
    endif()

    if(WIN32 AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/assets/BatView.ico)
        set(BATVIEW_WINDOWS_ICON ${CMAKE_CURRENT_SOURCE_DIR}/assets/BatView.ico)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/batView.rc.in
                       ${CMAKE_CURRENT_BINARY_DIR}/batView.rc
                       @ONLY)
        list(APPEND BATVIEW_APP_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/batView.rc)
    endif()
endif()

if(APPLE AND BATVIEW_ENABLE_WX)
    add_executable(batView MACOSX_BUNDLE ${BATVIEW_APP_SOURCES})
elseif(WIN32 AND BATVIEW_ENABLE_WX)
    add_executable(batView WIN32 ${BATVIEW_APP_SOURCES})
else()
    add_executable(batView ${BATVIEW_APP_SOURCES})
endif()
target_include_directories(batView PRIVATE src)
if(BATVIEW_EMBEDDED_PYTHON_ACTIVE)
    target_include_directories(batView PRIVATE "${BATVIEW_EMBEDDED_PYTHON_INCLUDE}")
endif()

if(BATVIEW_ENABLE_WX)
    target_compile_definitions(batView PRIVATE
        BATVIEW_ENABLE_WX=1
    )
    target_link_libraries(batView PRIVATE ${wxWidgets_LIBRARIES})
    if(BATVIEW_EMBEDDED_PYTHON_ACTIVE)
        target_compile_definitions(batView PRIVATE
            BATVIEW_ENABLE_EMBEDDED_PYTHON=1
            BATVIEW_EMBEDDED_PYTHON_HOME="${BATVIEW_EMBEDDED_PYTHON_ROOT}"
            BATVIEW_EMBEDDED_PYTHON_LIB="${BATVIEW_EMBEDDED_PYTHON_LIB}"
            BATVIEW_EMBEDDED_PYTHON_SITE_PACKAGES="${BATVIEW_EMBEDDED_PYTHON_SITE_PACKAGES}"
        )
        target_link_libraries(batView PRIVATE "${BATVIEW_EMBEDDED_PYTHON_LIBRARY}")
    endif()

    if(APPLE)
        set_target_properties(batView PROPERTIES
            MACOSX_BUNDLE_BUNDLE_NAME "batView"
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.roi-ri.batView"
            MACOSX_BUNDLE_ICON_FILE "BatView.png"
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Info.plist.in"
        )
        set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/assets/BatView.png PROPERTIES
            MACOSX_PACKAGE_LOCATION Resources
        )
        target_sources(batView PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/assets/BatView.png)
        set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/assets/batView.gif PROPERTIES
            MACOSX_PACKAGE_LOCATION Resources
        )
        target_sources(batView PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/assets/batView.gif)
        if(TARGET batView_icon)
            set_target_properties(batView PROPERTIES
                MACOSX_BUNDLE_ICON_FILE "BatView.icns"
            )
            set_source_files_properties(${BATVIEW_ICNS_FILE} PROPERTIES
                MACOSX_PACKAGE_LOCATION Resources
            )
            target_sources(batView PRIVATE ${BATVIEW_ICNS_FILE})
            add_dependencies(batView batView_icon)
        endif()
    endif()
endif()

if(BATVIEW_ENABLE_WX)
    if(APPLE)
        install(TARGETS batView BUNDLE DESTINATION .)
    else()
        install(TARGETS batView RUNTIME DESTINATION .)
        install(
            FILES
                "${CMAKE_CURRENT_SOURCE_DIR}/assets/BatView.png"
                "${CMAKE_CURRENT_SOURCE_DIR}/assets/batView.gif"
            DESTINATION assets
        )
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/BatView.ico")
            install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/BatView.ico" DESTINATION assets)
        endif()
    endif()
else()
    install(TARGETS batView RUNTIME DESTINATION .)
endif()

if(BATVIEW_BUILD_TESTS)
    add_subdirectory(external/googletest)

    add_executable(run_tests
        tests/ConnectionServiceTest.cpp
        tests/AcquisitionServiceTest.cpp
        tests/ExportServiceTest.cpp
        tests/PlotServiceTest.cpp
        ${BATVIEW_CORE_SOURCES}
    )

    target_include_directories(run_tests PRIVATE src)
    target_link_libraries(run_tests PRIVATE gtest gtest_main)
    if(BATVIEW_EMBEDDED_PYTHON_ACTIVE)
        target_include_directories(run_tests PRIVATE "${BATVIEW_EMBEDDED_PYTHON_INCLUDE}")
        target_compile_definitions(run_tests PRIVATE
            BATVIEW_ENABLE_EMBEDDED_PYTHON=1
            BATVIEW_EMBEDDED_PYTHON_HOME="${BATVIEW_EMBEDDED_PYTHON_ROOT}"
            BATVIEW_EMBEDDED_PYTHON_LIB="${BATVIEW_EMBEDDED_PYTHON_LIB}"
            BATVIEW_EMBEDDED_PYTHON_SITE_PACKAGES="${BATVIEW_EMBEDDED_PYTHON_SITE_PACKAGES}"
        )
        target_link_libraries(run_tests PRIVATE "${BATVIEW_EMBEDDED_PYTHON_LIBRARY}")
    endif()

    enable_testing()
    add_test(NAME BatViewTests COMMAND run_tests)
endif()

set(CPACK_PACKAGE_NAME "batView")
set(CPACK_PACKAGE_VENDOR "batView")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Desktop application for ESP32 battery test control")
set(CPACK_PACKAGE_CONTACT "support@batview.local")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DIRECTORY "${BATVIEW_DIST_DIR}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "batView")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    "${CMAKE_CURRENT_SOURCE_DIR}/docs/DOWNLOAD_AND_INSTALL.md"
    DESTINATION .
)

if(WIN32)
    set(CPACK_GENERATOR "ZIP")
elseif(APPLE)
    set(CPACK_GENERATOR "ZIP")
else()
    set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)
