cmake_minimum_required(VERSION 3.5)

set(TARGET_NAME FLAME)
project(${TARGET_NAME} VERSION 1.0.0 LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})



if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug" FORCE)
endif()



##################################
# Options: Library configuration #
##################################

add_subdirectory(modules/FLAME_Protocol)



################
# Dependencies #
################





#############################
# Static library definition #
#############################

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*")
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/include/*")
file(GLOB_RECURSE CONFIG_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/cmake/*")
list(APPEND CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt")
list(APPEND CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/conanfile.txt")

add_library(${TARGET_NAME} STATIC ${SRC_FILES} ${HEADER_FILES} ${CONFIG_FILES})
add_library(${TARGET_NAME}::${TARGET_NAME} ALIAS ${TARGET_NAME})

target_include_directories(${TARGET_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif ()

set_target_properties(${TARGET_NAME} PROPERTIES C_STANDARD 11)
set_target_properties(${TARGET_NAME} PROPERTIES CXX_EXTENSIONS OFF)

if(MSVC)
    set_property(TARGET ${TARGET_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    target_compile_options(${TARGET_NAME} PRIVATE -Wno-psabi)
endif()


############################
# Preprocessor definitions #
############################

if (WIN32)
    target_compile_definitions(${TARGET_NAME} PRIVATE
       WIN32_LEAN_AND_MEAN      # Prevents Windows.h from adding unnecessary includes
       NOMINMAX                 # Prevents Windows.h from defining min/max as macros 
       _CRT_SECURE_NO_WARNINGS
    )
endif()

target_compile_definitions(${TARGET_NAME} PUBLIC SPDLOG_COMPILED_LIB)



###########
# Linking #
###########

target_link_libraries(${TARGET_NAME} FLAME_Protocol::FLAME_Protocol)
target_link_libraries(${TARGET_NAME} pthread)



#######################
# Precompiled Headers #
#######################




#######
# IDE #
#######

#set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "src" FILES ${SRC_FILES})
#source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "include" FILES ${HEADER_FILES})
#source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${CONFIG_FILES})


##################
# Subdirectories #
##################




###########
# Install #
###########

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${TARGET_NAME}ConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# Install binaries
install(
    TARGETS ${TARGET_NAME}
    EXPORT ${TARGET_NAME}Targets
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    RUNTIME DESTINATION "bin"
    INCLUDES DESTINATION "include"
)

# Install headers
install(
    DIRECTORY include/
    DESTINATION "include"
    FILES_MATCHING PATTERN "*.h*"
)

install(
    EXPORT ${TARGET_NAME}Targets 
    DESTINATION "lib/cmake/${TARGET_NAME}"
    NAMESPACE ${TARGET_NAME}::
)

# Install Targets
configure_file(cmake/${TARGET_NAME}Config.cmake.in ${TARGET_NAME}Config.cmake @ONLY)
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}Config.cmake"
          "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}ConfigVersion.cmake"
    DESTINATION lib/cmake/${TARGET_NAME}
)
