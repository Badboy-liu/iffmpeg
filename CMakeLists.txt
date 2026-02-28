cmake_minimum_required(VERSION 3.31)
project(iffmpeg)

set(CMAKE_CXX_STANDARD 20)
find_package(FFMPEG   REQUIRED)
#find_package(PNG REQUIRED)
message(STATUS "FFMPEG_LIBRARIES = ${FFMPEG_LIBRARIES}")
message(STATUS "FFMPEG_INCLUDE_DIRS = ${FFMPEG_INCLUDE_DIRS}")
add_executable(${PROJECT_NAME} main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARIES})
#target_link_libraries(${PROJECT_NAME} PRIVATE avcodec-62.dll avdevice-62.dll avfilter-11.dll avformat-62.dll avutil-60.dll swresample-6.dll swscale-9.dll)


add_executable(${PROJECT_NAME}2 01/main.cpp)
#target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)
target_include_directories(${PROJECT_NAME}2 PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME}2 PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME}2 PRIVATE ${FFMPEG_LIBRARIES})


add_executable(${PROJECT_NAME}3 02/main.cpp)
#target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)
target_include_directories(${PROJECT_NAME}3 PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME}3 PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME}3 PRIVATE ${FFMPEG_LIBRARIES})

add_executable(${PROJECT_NAME}4 03/main.cpp)
#target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)
target_include_directories(${PROJECT_NAME}4 PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME}4 PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME}4 PRIVATE ${FFMPEG_LIBRARIES})


add_executable(${PROJECT_NAME}5 04/main.cpp)
#target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)
target_include_directories(${PROJECT_NAME}5 PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME}5 PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME}5 PRIVATE ${FFMPEG_LIBRARIES})



# 添加头文件搜索路径
#target_include_directories(${PROJECT_NAME} PRIVATE external/include)

# 添加库文件搜索路径
#target_link_directories(${PROJECT_NAME} PRIVATE external/lib)

## 链接导入库（.lib）
#target_link_libraries(${PROJECT_NAME} PRIVATE avcodec-62.dll avdevice-62.dll avfilter-11.dll avformat-62.dll avutil-60.dll swresample-6.dll swscale-9.dll)
## 在 CMakeLists.txt 末尾添加：
#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${CMAKE_SOURCE_DIR}/external/lib/avcodec-62.dll"
#        "${CMAKE_SOURCE_DIR}/external/lib/avdevice-62.dll"
#        "${CMAKE_SOURCE_DIR}/external/lib/avfilter-11.dll"
#        "${CMAKE_SOURCE_DIR}/external/lib/avformat-62.dll"
#        "${CMAKE_SOURCE_DIR}/external/lib/avutil-60.dll"
#        "${CMAKE_SOURCE_DIR}/external/lib/swresample-6.dll"
#        "${CMAKE_SOURCE_DIR}/external/lib/swscale-9.dll"
#        $<TARGET_FILE_DIR:${PROJECT_NAME}>
#)