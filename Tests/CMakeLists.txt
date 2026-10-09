# Tests/CMakeLists.txt

add_executable(FoundryTests
        Main.cpp
)

target_link_libraries(FoundryTests
        PRIVATE
        Core
)

add_test(
        NAME FoundryTests
        COMMAND FoundryTests
)