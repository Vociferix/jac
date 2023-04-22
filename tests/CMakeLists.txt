include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.0.1
)

FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
include(CTest)
include(Catch)

add_executable(tests
    holder.cpp
)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain jac::jac)
catch_discover_tests(tests)
