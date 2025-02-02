set(EXECUTABLE_NAME FinalAlert2YR)

find_package(WIL CONFIG REQUIRED)

file(GLOB_RECURSE SRC_FILES *.cpp)
file(GLOB_RECURSE RSC_FILES *.rc)

add_executable(${EXECUTABLE_NAME} WIN32 ${SRC_FILES} ${RSC_FILES})

# Shared MFC library: https://cmake.org/cmake/help/latest/variable/CMAKE_MFC_FLAG.html
target_compile_definitions(${EXECUTABLE_NAME} PRIVATE _AFXDLL)
set(CMAKE_MFC_FLAG 2)

# PCH
# target_precompile_headers(${EXECUTABLE_NAME} PRIVATE "stdafx.h")
set_target_properties(${EXECUTABLE_NAME} PROPERTIES COMPILE_FLAGS
	"/Yustdafx.h /FIstdafx.h /Fpstdafx.pch")
set_source_files_properties("stdafx.cpp" PROPERTIES COMPILE_FLAGS
	"/Ycstdafx.h")

target_link_libraries(${EXECUTABLE_NAME} PRIVATE
	ddraw
	dxguid
	winmm
	WIL::WIL
	PackLib_static
)