
file(GLOB_RECURSE SOURCEFILES "*.h" "*.hpp" "*.cpp")

add_library(CppUI STATIC ${SOURCEFILES})
