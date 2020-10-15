project(vrconduct)
cmake_minimum_required(VERSION 3.10)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB SRC src/*.cpp src/heatsink/*.cpp src/helper/*.cpp)
add_executable(main ${SRC})
target_include_directories(main PRIVATE include)

# Set references for windows
if (WIN32)
	# Get the architecture for library inclusion
	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(ARCH "64")
	elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
		set(ARCH "32")
	endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

	set(glm_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/glm/build")
	set(assimp_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/assimp/lib/cmake/assimp-4.1")
	set(GLFW_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/glfw/")
	set(stb_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/stb")
	set(OPENVR_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/openvr")
	set(OPENAL_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/openal-soft/bin")

	# GLEW / OpenAL do not proviude a nicer way of finding them :(
	set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/glew")
	list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/openal-soft")
	if (ARCH STREQUAL "32")
		add_compile_definitions(/LARGEADDRESSAWARE)

		set(CMAKE_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/glew/lib/Release/Win32/")
		set(GLEW_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/glew/bin/Release/Win32/")
		set(OPENVR_BIN_DIR "${OPENVR_ROOT_DIR}/bin/win32/")
	elseif (ARCH STREQUAL "64")
		set(CMAKE_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/glew/lib/Release/x64/")
		set(GLEW_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/glew/bin/Release/x64/")
		set(OPENVR_BIN_DIR "${OPENVR_ROOT_DIR}/bin/win64/")
	endif ()
endif (WIN32)

# Find needed packages
find_package(glm REQUIRED)
target_include_directories(main PRIVATE ${GLM_INCLUDE_DIRS})

find_package(assimp REQUIRED)
target_include_directories(main PRIVATE ${ASSIMP_INCLUDE_DIRS})
target_link_libraries(main ${ASSIMP_LIBRARY_DIRS}/${ASSIMP_LIBRARIES}.lib) # This can't be this dumb
target_compile_definitions(main PRIVATE ${ASSIMP_CXX_FLAGS})

find_package(GLEW REQUIRED)
target_include_directories(main PRIVATE ${GLEW_INCLUDE_DIRS})
target_link_libraries(main ${GLEW_LIBRARIES})

find_package(GLFW REQUIRED)
target_include_directories(main PRIVATE ${GLFW_INCLUDE_DIR})
target_link_libraries(main ${GLFW_LIBRARIES})

find_package(stb REQUIRED)
target_include_directories(main PRIVATE ${STB_INCLUDE_DIRS})

find_package(OpenVR REQUIRED)
target_include_directories(main PRIVATE ${OPENVR_INCLUDE_DIRS})
target_link_libraries(main ${OPENVR_LIBRARY})

find_package(OpenGL REQUIRED)
target_include_directories(main PRIVATE ${OPENGL_INCLUDE_DIRS})
target_link_libraries(main ${OPENGL_LIBRARIES})

find_package(OpenAL REQUIRED)
get_filename_component(TMP ${OPENAL_INCLUDE_DIR} DIRECTORY)
target_include_directories(main PRIVATE ${TMP})
target_link_libraries(main ${OPENAL_LIBRARY})
unset(TMP)

find_package(Threads REQUIRED)
target_link_libraries(main ${CMAKE_THREAD_LIBS_INIT})

find_package(SteamAudioAPI REQUIRED)
target_include_directories(main PRIVATE ${STEAM_AUDIO_INCLUDE_DIRS})
target_link_libraries(main ${STEAM_AUDIO_LIBS})

find_package(TinySoundFont REQUIRED)
target_include_directories(main PRIVATE ${TSF_INCLUDE_DIR})

# Copy DLLs/libs over for windows
if (WIN32)
	list(APPEND DLLS
		"${ASSIMP_ROOT_DIR}/bin/${ASSIMP_LIBRARIES}.dll"
		"${GLFW_LOCATION}/lib/glfw3.dll"
		"${OPENVR_BIN_DIR}/openvr_api.dll"
		"${GLEW_BIN_DIR}/glew32.dll"
		# "${OPENAL_BIN_DIR}/OpenAL32.dll"
		"${STEAM_AUDIO_BIN}")

	foreach (DLL IN LISTS DLLS)
		get_filename_component(DLL_PATH ${DLL} DIRECTORY)
		get_filename_component(DLL_NAME ${DLL} NAME)
		
		message(STATUS "Looking for DLL '${DLL_NAME}'")
		find_file(DLL_${DLL_NAME}_LOCATION NAME ${DLL_NAME} PATHS ${DLL_PATH})
		if (NOT DLL_${DLL_NAME}_LOCATION)
			message(FATAL_ERROR "DLL ${DLL_NAME} NOT found")
		else ()
			message(STATUS "Looking for DLL '${DLL_NAME}' - done (${DLL_${DLL_NAME}_LOCATION})")
			list(APPEND cp_list ${DLL_${DLL_NAME}_LOCATION})
		endif ()

		unset(DLL_PATH)
		unset(DLL_NAME)
	endforeach ()
	add_custom_command(TARGET main POST_BUILD        # Adds a post-build event to MyTest
		COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
			${cp_list}                    # <--this is in-file
			$<TARGET_FILE_DIR:main>)
endif (WIN32)