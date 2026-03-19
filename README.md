练习ffmpeg


### 环境步骤

1.visual studio

2.clion

3.vcpkg

4.安装ffmpeg
vcpkg install ffmpeg[all]



yuv播放器
在software文件夹下的[yuvplayer-2.5.zip](software/yuvplayer-2.5.zip)
打开软件修改size为592*1280


### 错误得处理方法:

- 1.This application failed to start because no Qt platform plugin could beinitialized. Reinstalling the application may fix this problem.
(Press Retry to debug the application)

这个是qt有个插件目录,下面命令就是把qt下得platforms复制到exe下的目录
```cmake
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
```


- 2.要安装ffmpeg全部依赖,和qt的依赖
 ```c++
    vcpkg install ffmpeg[all]
    vcpkg install qtbase qtdeclarative qttools qtsvg qtimageformats  qtshadertools qtquickcontrols2 --triplet=x64-windows
 ```
