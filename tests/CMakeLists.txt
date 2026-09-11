# ----====---- GoogleTest ----====---
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# ----====---- End: GoogleTest ----====---

add_executable(
  mimir_test
  "region_test.cpp"
  "arena_test.cpp"
  "test_util.cpp")
target_link_libraries(
  mimir_test
  GTest::gtest_main
  mimir
)

target_include_directories(mimir_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

include(GoogleTest)
gtest_discover_tests(mimir_test)
