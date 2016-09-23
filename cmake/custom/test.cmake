enable_testing()

add_test(test_main py.test -vv -s ${PROJECT_SOURCE_DIR}/test/test.py)

set_property(
    TEST
        test_main
    PROPERTY
        ENVIRONMENT PROJECT_BUILD_DIR=${PROJECT_BINARY_DIR}
    )

set_property(
    TEST
        test_main
    APPEND
    PROPERTY
        ENVIRONMENT PROJECT_INCLUDE_DIR=${PROJECT_SOURCE_DIR}/api
    )

set_property(
    TEST
        test_main
    APPEND
    PROPERTY
        ENVIRONMENT PYTHONPATH=${PROJECT_SOURCE_DIR}/api
    )

add_test(test_generate py.test -vv -s ${PROJECT_SOURCE_DIR}/src/generate.py)
