cmake_minimum_required(VERSION 3.31)
project(iffmpeg)

set(CMAKE_CXX_STANDARD 20)
find_package(FFMPEG   REQUIRED)
#find_package(PNG REQUIRED)
message(STATUS "FFMPEG_LIBRARIES = ${FFMPEG_LIBRARIES}")
message(STATUS "FFMPEG_INCLUDE_DIRS = ${FFMPEG_INCLUDE_DIRS}")
add_executable(${PROJECT_NAME} main.cpp
        07/I420render2.cpp
        07/I420render2.h)
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


add_executable(${PROJECT_NAME}6 05/main.cpp)
#target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)
target_include_directories(${PROJECT_NAME}6 PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME}6 PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME}6 PRIVATE ${FFMPEG_LIBRARIES})

add_executable(${PROJECT_NAME}7 05/main2.cpp)
#target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)
target_include_directories(${PROJECT_NAME}7 PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME}7 PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME}7 PRIVATE ${FFMPEG_LIBRARIES})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt6 COMPONENTS
        Core Widgets Gui    OpenGL Qml  Quick
        QuickControls2
        OpenGLWidgets OpenGL
        REQUIRED)

add_executable(${PROJECT_NAME}8 06/main.cpp 06/mainwindow.cpp
        06/mainwindow.h
        06/ffmpegwidget.cpp
        06/ffmpegwidget.h)
#target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)
target_include_directories(${PROJECT_NAME}8 PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME}8 PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME}8 PRIVATE ${FFMPEG_LIBRARIES})
target_link_libraries(${PROJECT_NAME}8 PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Gui
        ${FFMPEG_LIBRARIES}
        swscale
)
qt_finalize_executable(${PROJECT_NAME}8)


add_executable(${PROJECT_NAME}9 07/main.cpp 07/mainwindow.cpp
        07/mainwindow.h
        07/ffmpegdecoder.cpp
        07/ffmpegdecoder.h
        07/I420render.cpp
        07/I420render.h
        07/I420render2.cpp
        07/I420render2.h)
#target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)
target_include_directories(${PROJECT_NAME}9 PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME}9 PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME}9 PRIVATE ${FFMPEG_LIBRARIES})
target_link_libraries(${PROJECT_NAME}9 PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Gui
        ${FFMPEG_LIBRARIES}
        swscale
        Qt::OpenGL
        Qt::OpenGLWidgets
)
qt_finalize_executable(${PROJECT_NAME}9)

add_executable(${PROJECT_NAME}10 08/main.cpp 08/mainwindow.cpp
        08/mainwindow.h
        08/ffmpegdecoder.cpp
        08/ffmpegdecoder.h
        08/I420render.cpp
        08/I420render.h
        08/I420render2.cpp
        08/I420render2.h)
#target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)
target_include_directories(${PROJECT_NAME}10 PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME}10 PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME}10 PRIVATE ${FFMPEG_LIBRARIES})
target_link_libraries(${PROJECT_NAME}10 PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Gui
        ${FFMPEG_LIBRARIES}
        swscale
        Qt::OpenGL
        Qt::OpenGLWidgets
)
qt_finalize_executable(${PROJECT_NAME}10)



add_executable(${PROJECT_NAME}11 09/main.cpp
        09/ffmpegdecoder.cpp
        09/ffmpegdecoder.h
        09/I420render.cpp
        09/I420render.h
        09/VideoItem.cpp
        09/VideoItem.h
       )
qt_add_resources(RESOURCES
        09/qml.qrc
)
target_sources(${PROJECT_NAME}11 PRIVATE ${RESOURCES})


#target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)
target_include_directories(${PROJECT_NAME}11 PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME}11 PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME}11 PRIVATE ${FFMPEG_LIBRARIES})
target_link_libraries(${PROJECT_NAME}11 PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Gui
        Qt::Qml
        Qt::Quick
        Qt::QuickControls2
        ${FFMPEG_LIBRARIES}
        swscale
        Qt::OpenGL
        Qt::OpenGLWidgets
)
qt_finalize_executable(${PROJECT_NAME}11)

if (MSVC)
    add_custom_command(TARGET ${PROJECT_NAME}11 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:${PROJECT_NAME}8>/platforms

            COMMAND ${CMAKE_COMMAND} -E copy
            #            "${_VCPKG_INSTALLED_DIR}/x64-windows/tools/Qt6/plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll"
            "${_VCPKG_INSTALLED_DIR}/x64-windows/$<$<CONFIG:Debug>:debug/>plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}8>/platforms"
    )
endif()




add_executable(${PROJECT_NAME}12 10/main.cpp
        10/MainWindow.cpp
        10/MainWindow.h
        10/ffmpegvideo.cpp
        10/ffmpegvideo.h)


#target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)
target_include_directories(${PROJECT_NAME}12 PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME}12 PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME}12 PRIVATE ${FFMPEG_LIBRARIES})

target_link_libraries(${PROJECT_NAME}12 PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Gui
        Qt::Qml
        Qt::Quick
        Qt::QuickControls2
        ${FFMPEG_LIBRARIES}
        swscale
        Qt::OpenGL
        Qt::OpenGLWidgets
)
qt_finalize_executable(${PROJECT_NAME}12)

if (MSVC)
    add_custom_command(TARGET ${PROJECT_NAME}12 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:${PROJECT_NAME}8>/platforms

            COMMAND ${CMAKE_COMMAND} -E copy
            #            "${_VCPKG_INSTALLED_DIR}/x64-windows/tools/Qt6/plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll"
            "${_VCPKG_INSTALLED_DIR}/x64-windows/$<$<CONFIG:Debug>:debug/>plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}8>/platforms"
    )
endif()



add_executable(${PROJECT_NAME}13 11/main.cpp
        11/ffmpegdecoder.cpp
        11/ffmpegdecoder.h
        11/nv12render.cpp
        11/nv12render.h
        11/VideoItem.cpp
        11/VideoItem.h)
qt_add_resources(RESOURCES
        11/qml.qrc
)
target_sources(${PROJECT_NAME}13 PRIVATE ${RESOURCES})

#target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)
target_include_directories(${PROJECT_NAME}13 PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME}13 PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME}13 PRIVATE ${FFMPEG_LIBRARIES})

target_link_libraries(${PROJECT_NAME}13 PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Gui
        Qt::Qml
        Qt::Quick
        Qt::QuickControls2
        ${FFMPEG_LIBRARIES}
        swscale
        Qt::OpenGL
        Qt::OpenGLWidgets
)
qt_finalize_executable(${PROJECT_NAME}13)

if (MSVC)
    add_custom_command(TARGET ${PROJECT_NAME}13 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:${PROJECT_NAME}8>/platforms

            COMMAND ${CMAKE_COMMAND} -E copy
            #            "${_VCPKG_INSTALLED_DIR}/x64-windows/tools/Qt6/plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll"
            "${_VCPKG_INSTALLED_DIR}/x64-windows/$<$<CONFIG:Debug>:debug/>plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}8>/platforms"
    )
endif()



find_package(whisper CONFIG REQUIRED)


add_executable(${PROJECT_NAME}100 100/main.cpp)
#target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)
target_include_directories(${PROJECT_NAME}100 PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME}100 PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME}100 PRIVATE ${FFMPEG_LIBRARIES})
target_link_libraries(${PROJECT_NAME}100 PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Gui
        ${FFMPEG_LIBRARIES}
        swscale
        Qt::OpenGL
        Qt::OpenGLWidgets
)
qt_finalize_executable(${PROJECT_NAME}100)

if (MSVC)
    add_custom_command(TARGET ${PROJECT_NAME}100 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:${PROJECT_NAME}8>/platforms

            COMMAND ${CMAKE_COMMAND} -E copy
            #            "${_VCPKG_INSTALLED_DIR}/x64-windows/tools/Qt6/plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll"
            "${_VCPKG_INSTALLED_DIR}/x64-windows/$<$<CONFIG:Debug>:debug/>plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}8>/platforms"
    )
endif()
target_link_libraries(${PROJECT_NAME}100 PRIVATE whisper)
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