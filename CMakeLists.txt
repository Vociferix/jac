cmake_minimum_required(VERSION 3.25.3)

project(jac
    VERSION 0.1.0
    LANGUAGES CXX
)

if("${PROJECT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    set(JAC_TOP_LEVEL ON)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    file(RELATIVE_PATH _REL_BINDIR "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
    if(UNIX)
        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E create_symlink "${_REL_BINDIR}/compile_commands.json" "${PROJECT_SOURCE_DIR}/compile_commands.json"
            OUTPUT_QUIET
            ERROR_QUIET
        )
    endif()
else()
    set(JAC_TOP_LEVEL OFF)
endif()

add_library(jac INTERFACE)
add_library(jac::jac ALIAS jac)
target_include_directories(jac INTERFACE "${PROJECT_SOURCE_DIR}/include")
target_compile_features(jac INTERFACE cxx_std_20)

option(JAC_TESTS "Enable tests for jac" ${JAC_TOP_LEVEL})
if(JAC_TESTS)
    enable_testing()
    add_subdirectory(tests EXCLUDE_FROM_ALL)
endif()

option(JAC_DOCS "Enable documentation for jac" ${JAC_TOP_LEVEL})
if(JAC_DOCS)
    find_package(Doxygen REQUIRED dot OPTIONAL_COMPONENTS mscgen dia)
    include(FetchContent)
    FetchContent_Declare(
        DoxygenAwesome
        GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css.git
        GIT_TAG v2.2.0
    )
    FetchContent_MakeAvailable(DoxygenAwesome)
    set(DOXYGEN_EXCLUDE_SYMBOLS detail)
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_DISABLE_INDEX NO)
    set(DOXYGEN_FULL_SIDEBAR NO)
    set(DOXYGEN_STRIP_FROM_PATH include)
    set(DOXYGEN_EXTRACT_STATIC YES)
    set(DOXYGEN_PREDEFINED DOXYGEN)
    set(DOXYGEN_HTML_EXTRA_STYLESHEET
        "${doxygenawesome_SOURCE_DIR}/doxygen-awesome.css"
        "${doxygenawesome_SOURCE_DIR}/doxygen-awesome-sidebar-only.css"
    )
    set(DOXYGEN_HTML_COLORSTYLE LIGHT)
    doxygen_add_docs(docs "${PROJECT_SOURCE_DIR}/include/jac")
endif()

if(JAC_TOP_LEVEL)
    find_program(CLANG_FORMAT clang-format)
    file(GLOB_RECURSE FMT_FILES RELATIVE "${PROJECT_SOURCE_DIR}" include/*.hpp tests/*.cpp tests/*.hpp)
    file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/fmt")
    set(FMT_TAGS)
    foreach(FMT_FILE ${FMT_FILES})
        set(FMT_TAG "${PROJECT_BINARY_DIR}/fmt/${FMT_FILE}.tag")
        get_filename_component(FMT_TAG_DIR "${FMT_TAG}" DIRECTORY)
        list(APPEND FMT_TAGS "${FMT_TAG}")
        add_custom_command(OUTPUT "${FMT_TAG}"
            COMMAND "${CLANG_FORMAT}" -style=file -i "${PROJECT_SOURCE_DIR}/${FMT_FILE}"
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${FMT_TAG_DIR}"
            COMMAND "${CMAKE_COMMAND}" -E touch "${FMT_TAG}"
            COMMENT "Formatting ${FMT_FILE}"
        )
    endforeach()
    add_custom_target(format DEPENDS ${FMT_TAGS})
endif()
