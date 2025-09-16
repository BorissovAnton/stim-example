cmake_minimum_required(VERSION 3.20)
project(StimExample)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set up local third_party directory for caching external dependencies
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)
file(MAKE_DIRECTORY ${THIRD_PARTY_DIR})

# Function to handle dependency with local cache fallback
function(add_cached_dependency dep_name git_repo git_tag)
  set(dep_dir ${THIRD_PARTY_DIR}/${dep_name})

  # Check if local copy exists and has the expected git tag
  if(EXISTS ${dep_dir}/.git)
    # Verify the cached version matches the expected tag/commit
    execute_process(
      COMMAND git rev-parse HEAD
      WORKING_DIRECTORY ${dep_dir}
      OUTPUT_VARIABLE cached_commit
      OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)

    # For commit hashes, check if the cached commit starts with the expected
    # hash
    if(${git_tag} MATCHES "^[0-9a-f]+")
      if(cached_commit MATCHES "^${git_tag}")
        message(STATUS "Using cached ${dep_name} (${git_tag}) from ${dep_dir}")
        FetchContent_Declare(${dep_name} SOURCE_DIR ${dep_dir})
        return()
      else()
        message(
          STATUS
            "Cached ${dep_name} commit mismatch (found ${cached_commit}, expected ${git_tag}), redownloading"
        )
        file(REMOVE_RECURSE ${dep_dir})
      endif()
    else()
      # For tags, try to check if the tag exists and matches
      execute_process(
        COMMAND git describe --tags HEAD
        WORKING_DIRECTORY ${dep_dir}
        OUTPUT_VARIABLE cached_tag
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)

      if(cached_tag STREQUAL ${git_tag})
        message(STATUS "Using cached ${dep_name} (${git_tag}) from ${dep_dir}")
        FetchContent_Declare(${dep_name} SOURCE_DIR ${dep_dir})
        return()
      else()
        message(
          STATUS
            "Cached ${dep_name} tag mismatch (found ${cached_tag}, expected ${git_tag}), redownloading"
        )
        file(REMOVE_RECURSE ${dep_dir})
      endif()
    endif()
  endif()

  # Download from GitHub
  message(STATUS "Downloading ${dep_name} (${git_tag}) from ${git_repo}")
  FetchContent_Declare(
    ${dep_name}
    GIT_REPOSITORY ${git_repo}
    GIT_TAG ${git_tag})
endfunction()

# Function to cache dependencies after they are populated
function(cache_dependency_after_populate dep_name)
  set(dep_dir ${THIRD_PARTY_DIR}/${dep_name})

  if(NOT EXISTS ${dep_dir}/.git)
    FetchContent_GetProperties(${dep_name})
    if(${dep_name}_POPULATED)
      execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${${dep_name}_SOURCE_DIR}
                ${dep_dir} COMMAND_ERROR_IS_FATAL ANY)
      message(STATUS "Cached ${dep_name} to ${dep_dir}")
    endif()
  endif()
endfunction()

include(FetchContent)

# Handle stim dependency with caching
add_cached_dependency(stim https://github.com/BorissovAnton/stim.git 20788934db50e1ae110f6d9335b3afc22a564341)
FetchContent_MakeAvailable(stim)

# Cache stim after population
cache_dependency_after_populate(stim)
set_target_properties(stim PROPERTIES EXCLUDE_FROM_ALL ON)
set_target_properties(stim_perf PROPERTIES EXCLUDE_FROM_ALL ON)
if(TARGET stim_python_bindings)
  set_target_properties(stim_python_bindings PROPERTIES EXCLUDE_FROM_ALL ON)
endif()

add_executable(main main.cpp)
target_link_libraries(main PUBLIC libstim)
