find_package(WIL CONFIG REQUIRED)

set(TARGET_NAME PackLib)

file(GLOB_RECURSE SRC_FILES *.cpp)

set(TARGET_PRIVATE_LIBRARIES WIL::WIL)

# static library
add_library(${TARGET_NAME}_static STATIC ${SRC_FILES})
target_link_libraries(${TARGET_NAME}_static PRIVATE ${TARGET_PRIVATE_LIBRARIES})

add_library(${TARGET_NAME} SHARED ${SRC_FILES})
target_compile_definitions(${TARGET_NAME} PRIVATE BASE_EXPORT)
target_link_libraries(${TARGET_NAME}_static PRIVATE ${TARGET_PRIVATE_LIBRARIES})

