project(Demo CXX)

find_package(glm CONFIG REQUIRED)
find_package(Tracy CONFIG REQUIRED)
# find_package(spdlog CONFIG REQUIRED)

find_package(glew CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
set(VULKAN_SDK_PATH C:\\VulkanSDK\\1.3.280.0)

# 1. Set VULKAN_SDK_PATH in .env.cmake to target specific vulkan version
if (DEFINED VULKAN_SDK_PATH)
  set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include") # 1.1 Make sure this include path is correct
  set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib") # 1.2 Make sure lib path is correct
  set(Vulkan_FOUND "True")
else()
  find_package(Vulkan REQUIRED) # throws error if could not find Vulkan
  message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")
endif()
if (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Could not find Vulkan library!")
else()
	message(STATUS "Using vulkan lib at: ${Vulkan_LIBRARIES}")
endif()

set(TARGET Demo)

set(HDR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB HDRS
${HDR_DIR}/*.h
${HDR_DIR}/*.inl
${HDR_DIR}/resource/*.h
${HDR_DIR}/resource/*.inl
${HDR_DIR}/shader/*.h
${HDR_DIR}/rendering/*.h
${HDR_DIR}/rendering/*.inl

)


file(GLOB SRCS
${SRC_DIR}/*.cpp
${SRC_DIR}/resource/*.cpp
${SRC_DIR}/rendering/*.cpp
)

if(MSVC)
  add_definitions(-D_CONSOLE)
endif()

if(WIN32)
  add_definitions("-DVK_USE_PLATFORM_WIN32_KHR=1")
  add_definitions("-DNOMINMAX")
endif()

add_executable(${TARGET} ${HDRS} ${SRCS})

target_include_directories(${TARGET} PUBLIC
${HDR_DIR}
${HDR_DIR}/resource
${HDR_DIR}/shader
${HDR_DIR}/rendering
)

target_include_directories(${TARGET} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/3D-Graphics-Rendering-Cookbook")


target_include_directories(${TARGET} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PUBLIC ${Vulkan_LIBRARIES})


target_link_libraries(${TARGET} PRIVATE glfw GLEW::GLEW glm::glm vulkan-1 assimp::assimp tinyobjloader::tinyobjloader)


set_target_properties(${TARGET} PROPERTIES VS_GLOBAL_VcpkgEnabled true)

#set_target_properties(${TARGET} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set_target_properties(${TARGET} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

# add_definitions(-DSPDLOG_USE_STD_FORMAT=ON)

file(GLOB_RECURSE SHADER_HEAD ${HDR_DIR}/shader/*.h ${HDR_DIR}/shader/*.inl)
source_group("Header Files/shader" FILES ${SHADER_HEAD})

file(GLOB_RECURSE RESOURCE_HEAD ${HDR_DIR}/resource/*.h ${HDR_DIR}/resource/*.inl)
file(GLOB_RECURSE RESOURCE_SRC ${SRC_DIR}/resource/*.cpp)
source_group("Header Files/resource" FILES ${RESOURCE_HEAD})
source_group("Source Files/resource" FILES ${RESOURCE_SRC})

file(GLOB_RECURSE RENDERING_HEAD ${HDR_DIR}/rendering/*.h ${HDR_DIR}/rendering/*.inl)
file(GLOB_RECURSE RENDERING_SRC ${SRC_DIR}/rendering/*.cpp)
source_group("Header Files/rendering" FILES ${RENDERING_HEAD})
source_group("Source Files/rendering" FILES ${RENDERING_SRC})

#file(GLOB_RECURSE PANELS_HEAD ${HDR_DIR}/panels/*.h ${HDR_DIR}/panels/*.inl)
#file(GLOB_RECURSE PANELS_SRC ${SRC_DIR}/panels/*.cpp)
#source_group("Header Files/panels" FILES ${PANELS_HEAD})
#source_group("Source Files/panels" FILES ${PANELS_SRC})
