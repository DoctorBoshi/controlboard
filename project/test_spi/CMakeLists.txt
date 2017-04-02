add_executable(${CMAKE_PROJECT_NAME}
    test_openrtos.c
    test_spi.c
    )
    
target_link_libraries(${CMAKE_PROJECT_NAME}    
	spi
	)
# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)
