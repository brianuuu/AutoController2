cmake_minimum_required(VERSION 3.16)

project(AutoController2 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#dependencies
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS SerialPort)
find_package(Qt6 REQUIRED COMPONENTS Multimedia)
find_package(Qt6 REQUIRED COMPONENTS Concurrent)

set(LibVLC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/External/LibVLC")
set(fftw_DIR "${CMAKE_CURRENT_SOURCE_DIR}/External/fftw")

#dependencies directory headers
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${LibVLC_DIR}/include)
INCLUDE_DIRECTORIES(${fftw_DIR}/include)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/resource.rc")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(AutoController2
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${app_icon_resource_windows}
        Helpers/audioconversionutils.cpp Helpers/audioconversionutils.h
        Helpers/jsonhelper.h Helpers/jsonhelper.cpp
        Helpers/mediadiscoverer.h Helpers/mediadiscoverer.cpp
        Helpers/serialholder.h Helpers/serialholder.cpp
        Helpers/stickpainter.h Helpers/stickpainter.cpp
        Managers/audiomanager.h Managers/audiomanager.cpp
        Managers/joystickmanager.h Managers/joystickmanager.cpp
        Managers/keyboardmanager.h Managers/keyboardmanager.cpp
        Managers/logmanager.h Managers/logmanager.cpp
        Managers/managercollection.h
        Managers/programmanager.h Managers/programmanager.cpp
        Managers/serialmanager.h Managers/serialmanager.cpp
        Managers/videomanager.h Managers/videomanager.cpp
        Managers/vlcmanager.h Managers/vlcmanager.cpp
        Programs/Modules/Common/runcommand.h Programs/Modules/Common/runcommand.cpp
        Programs/Modules/modulebase.h Programs/Modules/modulebase.cpp
        Programs/Settings/settingbase.h
        Programs/Settings/settingcombobox.h Programs/Settings/settingcombobox.cpp
        Programs/Settings/settinglineedit.h Programs/Settings/settinglineedit.cpp
        Programs/Settings/settingtextbrowser.h Programs/Settings/settingtextbrowser.cpp
        Programs/Settings/settingtextedit.h Programs/Settings/settingtextedit.cpp
        Programs/System/commandrecorder.h Programs/System/commandrecorder.cpp
        Programs/System/customcommand.h Programs/System/customcommand.cpp
        Programs/programbase.h Programs/programbase.cpp
        Types/system.h
        defines.h
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET AutoController2 APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(AutoController2 SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(AutoController2
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(AutoController2 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(AutoController2 PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(AutoController2 PRIVATE Qt6::SerialPort)
target_link_libraries(AutoController2 PRIVATE Qt6::Multimedia)
target_link_libraries(AutoController2 PRIVATE Qt6::Concurrent)
target_link_libraries(AutoController2 PRIVATE ${LibVLC_DIR}/libvlc.lib)
target_link_libraries(AutoController2 PRIVATE ${LibVLC_DIR}/libvlccore.lib)
target_link_libraries(AutoController2 PRIVATE ${fftw_DIR}/libfftw3f-3.lib)
target_link_libraries(AutoController2 PRIVATE winmm)

#copy files and folders
add_custom_command(
    TARGET AutoController2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${LibVLC_DIR}/libvlc.dll" "${CMAKE_BINARY_DIR}/libvlc.dll"
)
add_custom_command(
    TARGET AutoController2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${LibVLC_DIR}/libvlccore.dll" "${CMAKE_BINARY_DIR}/libvlccore.dll"
)
add_custom_command(
    TARGET AutoController2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${fftw_DIR}/libfftw3f-3.dll" "${CMAKE_BINARY_DIR}/libfftw3f-3.dll"
)
add_custom_target(
    LibVLC_COPY_PLUGINS
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
    "${LibVLC_DIR}/plugins"
    "${CMAKE_BINARY_DIR}/plugins"
)
add_dependencies(AutoController2 LibVLC_COPY_PLUGINS)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.AutoController2)
endif()
set_target_properties(AutoController2 PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS AutoController2
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(AutoController2)
endif()
