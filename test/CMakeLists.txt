add_executable(storage_test
    storage/storage_test.cpp
)

target_link_libraries(storage_test
    PRIVATE
    ticket_system_lib
    gtest
    gtest_main
)

add_test(NAME storage_test COMMAND storage_test)

# Individual test cases
foreach(test_num RANGE 1 6)
    add_executable(storage_test_case${test_num} storage/storage_test.cpp)
    
    target_link_libraries(storage_test_case${test_num}
        PRIVATE
        ticket_system_lib
        gtest
        gtest_main
    )
    
    # Filter to run only the specified test case
    target_compile_definitions(storage_test_case${test_num} 
        PRIVATE 
        TEST_CASE_FILTER="StorageTest.TestCase${test_num}"
    )
    
    add_test(NAME storage_test_case${test_num} 
             COMMAND storage_test_case${test_num}
             --gtest_filter=StorageTest.TestCase${test_num})
endforeach()