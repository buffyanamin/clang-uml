cmake_minimum_required(VERSION 3.16)

#
# Project name
#
project(clang-uml C CXX)

#
# CMake standard defines
#
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_FIND_DEBUG_MODE OFF)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

#
# C++ setup
#
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#
# clang-uml custom defines
#
set(CLANG_UML_INSTALL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(CLANG_UML_INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
set(UML_HEADERS_DIR ${PROJECT_SOURCE_DIR}/src/uml)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

#
# CMake build options
#
option(LINK_LLVM_SHARED "Should LLVM be linked using shared libraries" ON)
set(LLVM_VERSION CACHE STRING "Major LLVM version to use (e.g. 15)")
set(LLVM_CONFIG_PATH CACHE STRING "Path to llvm-config binary")
set(CMAKE_PREFIX CACHE STRING "Path to custom cmake modules")

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_PREFIX}")

#
# Setup version string
#
include(GitVersion)
setup_git_version()
message(STATUS "clang-uml version: "
               "${GIT_VERSION_MAJOR}.${GIT_VERSION_MINOR}.${GIT_VERSION_PATCH}")

#
# Setup coverage
#
option(CODE_COVERAGE "" OFF)
if(CODE_COVERAGE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g --coverage -fno-inline")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g --coverage -fno-inline")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov")
endif(CODE_COVERAGE)

#
# Setup Address Sanitizer
#
option(ADDRESS_SANITIZER "" OFF)
if(ADDRESS_SANITIZER)
    message(STATUS "Enabling address sanitizer")
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} \
            -fno-omit-frame-pointer \
            -fsanitize=address \
            -fsanitize-blacklist=${CMAKE_CURRENT_SOURCE_DIR}/.sanitize-blacklist"
    )
    set(CMAKE_EXE_LINKER_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS} \
            -fno-omit-frame-pointer \
            -fsanitize=address \
            -fsanitize-blacklist=${CMAKE_CURRENT_SOURCE_DIR}/.sanitize-blacklist"
    )
endif(ADDRESS_SANITIZER)

#
# Setup LLVM
#
include(LLVMSetup)

#
# Setup custom compile options depending on various compiler and environment
# quirks
#
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "13.0")
        # Workaround over Wdangling-reference false positives in libfmt
        set(CUSTOM_COMPILE_OPTIONS ${CUSTOM_COMPILE_OPTIONS}
                                   -Wno-dangling-reference)
    endif()
endif()

if(LLVM_VERSION_MAJOR GREATER_EQUAL 17)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CUSTOM_COMPILE_OPTIONS ${CUSTOM_COMPILE_OPTIONS}
                                   -Wno-class-memaccess)
    endif()
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CUSTOM_COMPILE_OPTIONS ${CUSTOM_COMPILE_OPTIONS}
                               -Wno-unused-private-field)
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstandalone-debug")
    endif()
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19)
        set(CUSTOM_COMPILE_OPTIONS
            ${CUSTOM_COMPILE_OPTIONS}
            -Wno-missing-template-arg-list-after-template-kw)
    endif()
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 20)
        set(CMAKE_CXX_STANDARD 20)
        set(CUSTOM_COMPILE_OPTIONS ${CUSTOM_COMPILE_OPTIONS}
                                   -DFMT_USE_CONSTEXPR=0
                                   -DFMT_USE_CONSTEVAL=0
                                   -Wno-invalid-constexpr
                                   -Wno-deprecated-anon-enum-enum-conversion)
    endif()
endif()

link_directories(${LLVM_LIBRARY_DIR})

#
# Setup threads library
#
if(APPLE)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  set(CMAKE_THREAD_LIBS_INIT "-lpthread")
  set(CMAKE_HAVE_LIBC_PTHREAD 1)
endif(APPLE)
find_package(Threads REQUIRED)

#
# Setup yaml-cpp
#
message(STATUS "Checking for yaml-cpp...")
find_package(yaml-cpp REQUIRED)
if(MSVC)
    add_definitions(-DYAML_CPP_STATIC_DEFINE)
endif(MSVC)
if(APPLE)
    set(YAML_CPP_LIBRARIES yaml-cpp::yaml-cpp)
endif(APPLE)
message(STATUS "Found yaml-cpp at: ${YAML_CPP_INCLUDE_DIR}, library: ${YAML_CPP_LIBRARIES}")

#
# Setup backward-cpp, libdw and libunwind
#
if(APPLE)
    include(LibUnwindSetup)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif(APPLE)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CLANG_UML_ENABLE_BACKTRACE_DEFAULT ON)
else()
    set(CLANG_UML_ENABLE_BACKTRACE_DEFAULT OFF)
endif()
option(CLANG_UML_ENABLE_BACKTRACE "Enable backtrace on segfault"
       ${CLANG_UML_ENABLE_BACKTRACE_DEFAULT})
if(LINUX AND CLANG_UML_ENABLE_BACKTRACE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(DW REQUIRED IMPORTED_TARGET libdw)
    pkg_check_modules(UNWIND REQUIRED IMPORTED_TARGET libunwind)
    set(BACKWARD_CPP_LIBRARIES PkgConfig::DW PkgConfig::UNWIND)
    set(ENABLE_BACKWARD_CPP -DENABLE_BACKWARD_CPP)
    message(STATUS "Enabling backward-cpp")
else()
    message(STATUS "Disabling backward-cpp")
endif()

#
# Setup pugixml
#
if(MSVC)
    add_definitions(-DPUGIXML_HEADER_ONLY)
else(MSVC)
    add_subdirectory(thirdparty/pugixml)
endif(MSVC)

#
# Setup thirdparty sources
#
set(THIRDPARTY_HEADERS_DIR ${PROJECT_SOURCE_DIR}/thirdparty/)

#
# Setup include directories
#
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_UML_INSTALL_INCLUDE_DIR})
include_directories(${UML_HEADERS_DIR})
include_directories(${THIRDPARTY_HEADERS_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src/)
include_directories(${PROJECT_BINARY_DIR}/src/version)
include_directories(${YAML_CPP_INCLUDE_DIR})

add_subdirectory(src)

#
# Enable testing via CTest
#
option(BUILD_TESTS "" ON)
option(ENABLE_BENCHMARKS "" OFF)
option(ENABLE_CXX_MODULES_TEST_CASES "" OFF)
option(ENABLE_CUDA_TEST_CASES "" OFF)
option(ENABLE_OBJECTIVE_C_TEST_CASES "" OFF)
option(FETCH_LIBOBJC2 "" OFF)

#
# Setup CUDA if available
#
if(ENABLE_CUDA_TEST_CASES)
    include(CheckLanguage)
    check_language(CUDA)
    set(CMAKE_CUDA_STANDARD 17)
    set(CMAKE_CUDA_ARCHITECTURES 75)
    enable_language(CUDA)
endif(ENABLE_CUDA_TEST_CASES)

if(ENABLE_OBJECTIVE_C_TEST_CASES)
    enable_language(OBJC)
    if(UNIX AND NOT APPLE)
        find_package(GNUstep REQUIRED)
    endif()
endif(ENABLE_OBJECTIVE_C_TEST_CASES)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif(BUILD_TESTS)
