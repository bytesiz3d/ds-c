cmake_minimum_required(VERSION 3.20)

add_executable(unittests
    utest.h
    unittests.c
    unittests_array.c
    unittests_set.c
    unittests_allocator.c
)
target_link_libraries(unittests ds)
add_test(NAME unittests COMMAND unittests)

if (MSVC)
    target_compile_options(unittests PRIVATE /W4 /WX)
else ()
    target_compile_options(unittests PRIVATE -Wall -Wextra -Werror -Wno-missing-field-initializers)
endif ()