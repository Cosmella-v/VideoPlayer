cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(VideoPlayer VERSION 1.0.0)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    include
)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

if(WIN32)
    target_link_libraries(${PROJECT_NAME} opengl32)
elseif(IOS)
    target_link_libraries(${PROJECT_NAME} "-framework OpenGLES")
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
elseif(ANDROID)
    target_link_libraries(${PROJECT_NAME} GLESv2)
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)



setup_geode_mod(${PROJECT_NAME})

if (WIN32)
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/windows 
    )
    set(STATIC_LIBS 
        ${CMAKE_SOURCE_DIR}/lib/windows/avcodec.lib
        ${CMAKE_SOURCE_DIR}/lib/windows/avformat.lib
        ${CMAKE_SOURCE_DIR}/lib/windows/avutil.lib
        ${CMAKE_SOURCE_DIR}/lib/windows/avfilter.lib
        ${CMAKE_SOURCE_DIR}/lib/windows/swresample.lib
        ${CMAKE_SOURCE_DIR}/lib/windows/swscale.lib
        ${CMAKE_SOURCE_DIR}/lib/windows/libx264.lib
        ${CMAKE_SOURCE_DIR}/lib/windows/x265-static.lib
        ${CMAKE_SOURCE_DIR}/lib/windows/aom.lib
        ${CMAKE_SOURCE_DIR}/lib/windows/vpx.lib
        ${CMAKE_SOURCE_DIR}/lib/windows/openh264.lib
        ws2_32.lib
        mfplat.lib
        mf.lib
        secur32.lib
        crypt32.lib
        mfuuid.lib
        bcrypt.lib
        strmiids.lib
    ) # windows
elseif (ANDROID) 
    if (ANDROID_ABI STREQUAL "arm64-v8a")
        set(STATIC_LIBS
            ${CMAKE_SOURCE_DIR}/lib/arm64-v8a/libavcodec.a
            ${CMAKE_SOURCE_DIR}/lib/arm64-v8a/libavformat.a
            ${CMAKE_SOURCE_DIR}/lib/arm64-v8a/libavutil.a
            ${CMAKE_SOURCE_DIR}/lib/arm64-v8a/libavfilter.a
            ${CMAKE_SOURCE_DIR}/lib/arm64-v8a/libswresample.a
            ${CMAKE_SOURCE_DIR}/lib/arm64-v8a/libswscale.a
            ${CMAKE_SOURCE_DIR}/lib/arm64-v8a/libpostproc.a
            ${CMAKE_SOURCE_DIR}/lib/arm64-v8a/libaom.a
            ${CMAKE_SOURCE_DIR}/lib/arm64-v8a/libvpx.a
            ${CMAKE_SOURCE_DIR}/lib/arm64-v8a/libx264.a
            ${CMAKE_SOURCE_DIR}/lib/arm64-v8a/libx265.a
        ) # android
    elseif(ANDROID_ABI STREQUAL "armeabi-v7a")
        add_definitions(-D__USE_FILE_OFFSET64)
        add_definitions(-DANDROID_PLATFORM=24)
        set(STATIC_LIBS
            ${CMAKE_SOURCE_DIR}/lib/armeabi-v7a/libavcodec.a
            ${CMAKE_SOURCE_DIR}/lib/armeabi-v7a/libavformat.a
            ${CMAKE_SOURCE_DIR}/lib/armeabi-v7a/libavutil.a
            ${CMAKE_SOURCE_DIR}/lib/armeabi-v7a/libavfilter.a
            ${CMAKE_SOURCE_DIR}/lib/armeabi-v7a/libswresample.a
            ${CMAKE_SOURCE_DIR}/lib/armeabi-v7a/libswscale.a
            ${CMAKE_SOURCE_DIR}/lib/armeabi-v7a/libpostproc.a
            ${CMAKE_SOURCE_DIR}/lib/armeabi-v7a/libaom.a
            ${CMAKE_SOURCE_DIR}/lib/armeabi-v7a/libvpx.a
            ${CMAKE_SOURCE_DIR}/lib/armeabi-v7a/libx264.a
            ${CMAKE_SOURCE_DIR}/lib/armeabi-v7a/libx265.a
        ) # android
    endif()
else ()
    find_package(BZIP2 REQUIRED)
    set(STATIC_LIBS 
        "-framework Security"
        ${CMAKE_SOURCE_DIR}/lib/libx264.a
        ${CMAKE_SOURCE_DIR}/lib/libavcodec.a
        ${CMAKE_SOURCE_DIR}/lib/libavformat.a
        ${CMAKE_SOURCE_DIR}/lib/libavutil.a
        ${CMAKE_SOURCE_DIR}/lib/libavfilter.a
        ${CMAKE_SOURCE_DIR}/lib/libpostproc.a
        ${CMAKE_SOURCE_DIR}/lib/libswresample.a
        ${CMAKE_SOURCE_DIR}/lib/libswscale.a
        ${CMAKE_SOURCE_DIR}/lib/liblzma.a   
        ${BZIP2_LIBRARY}
        "-lm -lbz2 -lz -liconv -pthread -framework Security -framework VideoToolbox -framework CoreFoundation -framework CoreMedia -framework CoreVideo -framework CoreServices -framework CoreAudio -framework AVFoundation -framework CoreGraphics -framework AudioToolbox -framework Metal -framework CoreImage -framework AppKit"
    ) # macos
endif()
target_link_libraries(${PROJECT_NAME} ${STATIC_LIBS})

if (PROJECT_IS_TOP_LEVEL) 
    target_compile_definitions(${PROJECT_NAME} PRIVATE _VIDEO_PLAYER_EXPORTING)
endif()