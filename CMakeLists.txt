cmake_minimum_required(VERSION 3.14)
project(MyServer)

# using cpp 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# vcpkg required.
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake CACHE STRING "")
else()
    message(FATAL_ERROR "VCPKG_ROOT environment variable not set. Please set it to the path of your vcpkg installation.")
endif()


find_package(enet CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)


include_directories(${ENET_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR})

# Source files
set(SOURCES
    src/ddos_protection.cpp
    src/encryption.cpp
    src/main.cpp
    src/user_manager.cpp
)

# Header files
set(HEADERS
    src/ddos_protection.hpp
    src/encryption.hpp
    src/user_manager.hpp
)

# Add executable
add_executable(MyServer ${SOURCES} ${HEADERS})


target_link_libraries(MyServer PRIVATE enet OpenSSL::SSL nlohmann_json::nlohmann_json)