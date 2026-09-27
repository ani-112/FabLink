cmake_minimum_required(VERSION 3.21)

project(FabLink
    VERSION 1.0
    DESCRIPTION "SECS/GEM Semiconductor Equipment Communication Simulator"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

qt_add_executable(FabLink
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui

    src/common/types.cpp
    include/common/types.h

    src/equipment/equipment.cpp
    src/equipment/equipment_config.cpp
    src/equipment/measurement_worker.cpp
    include/equipment/equipment.h
    include/equipment/equipment_config.h
    include/equipment/measurement_worker.h

    src/secs/secs_item.cpp
    src/secs/secs_message.cpp
    include/secs/secs_item.h
    include/secs/secs_message.h

    src/hsms/hsms_message.cpp
    src/hsms/hsms_connection.cpp
    include/hsms/hsms_message.h
    include/hsms/hsms_connection.h

    src/gem/gem_state_machine.cpp
    src/gem/gem_event.cpp
    include/gem/gem_state.h
    include/gem/gem_state_machine.h
    include/gem/gem_event.h

    src/communication/communication_manager.cpp
    include/communication/communication_manager.h
)

target_include_directories(FabLink PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(FabLink PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

if(MSVC)
    target_compile_options(FabLink PRIVATE /W4 /permissive-)
else()
    target_compile_options(FabLink PRIVATE -Wall -Wextra -Wpedantic)
endif()


qt_add_executable(FabLinkHost
    host_main.cpp

    src/secs/secs_item.cpp
    src/secs/secs_message.cpp
    include/secs/secs_item.h
    include/secs/secs_message.h

    src/hsms/hsms_message.cpp
    src/hsms/hsms_connection.cpp
    include/hsms/hsms_message.h
    include/hsms/hsms_connection.h

    include/common/types.h
)

target_include_directories(FabLinkHost PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(FabLinkHost PRIVATE
    Qt6::Core
    Qt6::Network
)

if(MSVC)
    target_compile_options(FabLinkHost PRIVATE /W4 /permissive-)
else()
    target_compile_options(FabLinkHost PRIVATE -Wall -Wextra -Wpedantic)
endif()
