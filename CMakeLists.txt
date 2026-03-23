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
#target_link_libraries(${PROJECT_NAME} PRIVATE avcodec-62.dll avdevice-62.dll avfilter-11_qt_qml_gpu_paint.dll avformat-62.dll avutil-60.dll swresample-6.dll swscale-9.dll)


add_executable(${PROJECT_NAME}2 01_print_version/main.cpp
        constant.h)
#target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)
target_include_directories(${PROJECT_NAME}2 PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME}2 PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME}2 PRIVATE ${FFMPEG_LIBRARIES})


add_executable(${PROJECT_NAME}3 02_print_format/main.cpp
        constant.h)
#target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)
target_include_directories(${PROJECT_NAME}3 PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME}3 PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME}3 PRIVATE ${FFMPEG_LIBRARIES})

add_executable(${PROJECT_NAME}4 03_statistics_frame/main.cpp
        constant.h)
#target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)
target_include_directories(${PROJECT_NAME}4 PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME}4 PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME}4 PRIVATE ${FFMPEG_LIBRARIES})


add_executable(${PROJECT_NAME}5 04_save_rgb_frame/main.cpp
        constant.h)
#target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)
target_include_directories(${PROJECT_NAME}5 PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME}5 PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME}5 PRIVATE ${FFMPEG_LIBRARIES})


add_executable(${PROJECT_NAME}6 05_save_yuv_frame/main.cpp
        constant.h)
#target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)
target_include_directories(${PROJECT_NAME}6 PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME}6 PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME}6 PRIVATE ${FFMPEG_LIBRARIES})

add_executable(${PROJECT_NAME}7 05_save_yuv_frame/main2.cpp
        constant.h)
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

add_executable(${PROJECT_NAME}8 06_qt_paint/main.cpp 06_qt_paint/mainwindow.cpp
        06_qt_paint/mainwindow.h
        06_qt_paint/ffmpegwidget.cpp
        06_qt_paint/ffmpegwidget.h
        constant.h)
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
if (MSVC)
    add_custom_command(TARGET ${PROJECT_NAME}8 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:${PROJECT_NAME}8>/platforms

            COMMAND ${CMAKE_COMMAND} -E copy
            #            "${_VCPKG_INSTALLED_DIR}/x64-windows/tools/Qt6/plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll"
            "${_VCPKG_INSTALLED_DIR}/x64-windows/$<$<CONFIG:Debug>:debug/>/Qt6/plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}8>/platforms"
    )
endif()


add_executable(${PROJECT_NAME}9 07_qt_opengl_paint/main.cpp 07_qt_opengl_paint/mainwindow.cpp
        07_qt_opengl_paint/mainwindow.h
        07_qt_opengl_paint/ffmpegdecoder.cpp
        07_qt_opengl_paint/ffmpegdecoder.h
        07_qt_opengl_paint/I420render.cpp
        07_qt_opengl_paint/I420render.h
        07_qt_opengl_paint/I420render2.cpp
        07_qt_opengl_paint/I420render2.h
        constant.h)
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

add_executable(${PROJECT_NAME}10 08_qt_opengl_paint2/main.cpp 08_qt_opengl_paint2/mainwindow.cpp
        08_qt_opengl_paint2/mainwindow.h
        08_qt_opengl_paint2/ffmpegdecoder.cpp
        08_qt_opengl_paint2/ffmpegdecoder.h
        08_qt_opengl_paint2/I420render.cpp
        08_qt_opengl_paint2/I420render.h
        08_qt_opengl_paint2/I420render2.cpp
        08_qt_opengl_paint2/I420render2.h
        constant.h)
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



add_executable(${PROJECT_NAME}11 09_qt_qml_paint/main.cpp
        09_qt_qml_paint/ffmpegdecoder.cpp
        09_qt_qml_paint/ffmpegdecoder.h
        09_qt_qml_paint/I420render.cpp
        09_qt_qml_paint/I420render.h
        09_qt_qml_paint/VideoItem.cpp
        09_qt_qml_paint/VideoItem.h
        constant.h
)
qt_add_resources(RESOURCES
        09_qt_qml_paint/qml.qrc
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
            "${_VCPKG_INSTALLED_DIR}/x64-windows/$<$<CONFIG:Debug>:debug/>/Qt6/plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}8>/platforms"
    )
endif()




add_executable(${PROJECT_NAME}12 10_qt_gpu_paint/main.cpp
        10_qt_gpu_paint/MainWindow.cpp
        10_qt_gpu_paint/MainWindow.h
        10_qt_gpu_paint/ffmpegvideo.cpp
        10_qt_gpu_paint/ffmpegvideo.h
        constant.h)


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
            "${_VCPKG_INSTALLED_DIR}/x64-windows/$<$<CONFIG:Debug>:debug/>/Qt6/plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}8>/platforms"
    )
endif()



add_executable(${PROJECT_NAME}13 11_qt_qml_gpu_paint/main.cpp
        11_qt_qml_gpu_paint/ffmpegdecoder.cpp
        11_qt_qml_gpu_paint/ffmpegdecoder.h
        11_qt_qml_gpu_paint/nv12render.cpp
        11_qt_qml_gpu_paint/nv12render.h
        11_qt_qml_gpu_paint/VideoItem.cpp
        11_qt_qml_gpu_paint/VideoItem.h
        constant.h)
qt_add_resources(RESOURCES
        11_qt_qml_gpu_paint/qml.qrc
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
            "${_VCPKG_INSTALLED_DIR}/x64-windows/$<$<CONFIG:Debug>:debug/>/Qt6/plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}8>/platforms"
    )
endif()


add_executable(${PROJECT_NAME}14 12_mp4_to_h264/main.cpp
        constant.h)

#target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)
target_include_directories(${PROJECT_NAME}14 PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME}14 PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME}14 PRIVATE ${FFMPEG_LIBRARIES})

target_link_libraries(${PROJECT_NAME}14 PRIVATE
        ${FFMPEG_LIBRARIES}
        swscale
        Qt::Core
)






add_executable(${PROJECT_NAME}15 13_h264_to_mp4/main.cpp
        constant.h)


#target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)
target_include_directories(${PROJECT_NAME}15 PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME}15 PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME}15 PRIVATE ${FFMPEG_LIBRARIES})

target_link_libraries(${PROJECT_NAME}15 PRIVATE
        Qt::Core
        ${FFMPEG_LIBRARIES}
        swscale
)
qt_finalize_executable(${PROJECT_NAME}15)

if (MSVC)
    add_custom_command(TARGET ${PROJECT_NAME}15 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:${PROJECT_NAME}15>/platforms

            COMMAND ${CMAKE_COMMAND} -E copy
            #            "${_VCPKG_INSTALLED_DIR}/x64-windows/tools/Qt6/plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll"
            "${_VCPKG_INSTALLED_DIR}/x64-windows/$<$<CONFIG:Debug>:debug/>/Qt6/plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}15>/platforms"
    )
endif()



find_package(whisper CONFIG REQUIRED)


add_executable(${PROJECT_NAME}100 100/main.cpp
        constant.h)
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
            "${_VCPKG_INSTALLED_DIR}/x64-windows/$<$<CONFIG:Debug>:debug/>/Qt6/plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}100>/platforms"
    )
endif()
target_link_libraries(${PROJECT_NAME}100 PRIVATE whisper)
# 添加头文件搜索路径
#target_include_directories(${PROJECT_NAME} PRIVATE external/include)

# 添加库文件搜索路径
#target_link_directories(${PROJECT_NAME} PRIVATE external/lib)

## 链接导入库（.lib）
#target_link_libraries(${PROJECT_NAME} PRIVATE avcodec-62.dll avdevice-62.dll avfilter-11_qt_qml_gpu_paint.dll avformat-62.dll avutil-60.dll swresample-6.dll swscale-9.dll)
## 在 CMakeLists.txt 末尾添加：
#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${CMAKE_SOURCE_DIR}/external/lib/avcodec-62.dll"
#        "${CMAKE_SOURCE_DIR}/external/lib/avdevice-62.dll"
#        "${CMAKE_SOURCE_DIR}/external/lib/avfilter-11_qt_qml_gpu_paint.dll"
#        "${CMAKE_SOURCE_DIR}/external/lib/avformat-62.dll"
#        "${CMAKE_SOURCE_DIR}/external/lib/avutil-60.dll"
#        "${CMAKE_SOURCE_DIR}/external/lib/swresample-6.dll"
#        "${CMAKE_SOURCE_DIR}/external/lib/swscale-9.dll"
#        $<TARGET_FILE_DIR:${PROJECT_NAME}>
#)