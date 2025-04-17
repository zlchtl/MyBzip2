add_executable (CMakeProject1 "CMakeProject1.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CMakeProject1 PROPERTY CXX_STANDARD 20)
endif()

find_package(BZip2 REQUIRED)
target_link_libraries(CMakeProject1 PRIVATE BZip2::BZip2)