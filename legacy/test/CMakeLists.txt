enable_testing()
find_package(Catch2 REQUIRED)

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "*/test_*.cpp")
foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    file(RELATIVE_PATH REL_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${TEST_SOURCE})
    get_filename_component(DIR_PATH ${REL_PATH} DIRECTORY)

    string(REPLACE "/" ";" DIR_PARTS ${DIR_PATH})
    list(GET DIR_PARTS 1 LIB_NAME)
    list(GET DIR_PARTS 0 TEST_KIND)
    set(TEST_TARGET_NAME "${LIB_NAME}_${TEST_KIND}_${TEST_NAME}")

    add_executable(${TEST_TARGET_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_TARGET_NAME} PRIVATE ${LIB_NAME})
    target_link_libraries(${TEST_TARGET_NAME} PRIVATE Catch2::Catch2WithMain)
    add_test(NAME ${TEST_TARGET_NAME} COMMAND ${TEST_TARGET_NAME})
endforeach()
