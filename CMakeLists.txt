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
find_package(Qt6 REQUIRED COMPONENTS Network)
find_package(Qt6 REQUIRED COMPONENTS WebSockets)

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
        External/QDiscord/Discord.h
        External/QDiscord/Discord/Client.cpp External/QDiscord/Discord/Client.h
        External/QDiscord/Discord/GatewayEvents.cpp External/QDiscord/Discord/GatewayEvents.h
        External/QDiscord/Discord/GatewaySocket.cpp External/QDiscord/Discord/GatewaySocket.h
        External/QDiscord/Discord/HttpService.cpp External/QDiscord/Discord/HttpService.h
        External/QDiscord/Discord/Objects/Activity.cpp External/QDiscord/Discord/Objects/Activity.h
        External/QDiscord/Discord/Objects/Attachment.cpp External/QDiscord/Discord/Objects/Attachment.h
        External/QDiscord/Discord/Objects/Ban.cpp External/QDiscord/Discord/Objects/Ban.h
        External/QDiscord/Discord/Objects/Channel.cpp External/QDiscord/Discord/Objects/Channel.h
        External/QDiscord/Discord/Objects/Connection.cpp External/QDiscord/Discord/Objects/Connection.h
        External/QDiscord/Discord/Objects/Embed.cpp External/QDiscord/Discord/Objects/Embed.h
        External/QDiscord/Discord/Objects/Emoji.cpp External/QDiscord/Discord/Objects/Emoji.h
        External/QDiscord/Discord/Objects/Guild.cpp External/QDiscord/Discord/Objects/Guild.h
        External/QDiscord/Discord/Objects/GuildEmbed.cpp External/QDiscord/Discord/Objects/GuildEmbed.h
        External/QDiscord/Discord/Objects/GuildMember.cpp External/QDiscord/Discord/Objects/GuildMember.h
        External/QDiscord/Discord/Objects/Integration.cpp External/QDiscord/Discord/Objects/Integration.h
        External/QDiscord/Discord/Objects/Invite.cpp External/QDiscord/Discord/Objects/Invite.h
        External/QDiscord/Discord/Objects/Message.cpp External/QDiscord/Discord/Objects/Message.h
        External/QDiscord/Discord/Objects/Overwrite.cpp External/QDiscord/Discord/Objects/Overwrite.h
        External/QDiscord/Discord/Objects/PresenceUpdate.cpp External/QDiscord/Discord/Objects/PresenceUpdate.h
        External/QDiscord/Discord/Objects/Reaction.cpp External/QDiscord/Discord/Objects/Reaction.h
        External/QDiscord/Discord/Objects/Role.cpp External/QDiscord/Discord/Objects/Role.h
        External/QDiscord/Discord/Objects/User.cpp External/QDiscord/Discord/Objects/User.h
        External/QDiscord/Discord/Objects/VoiceRegion.cpp External/QDiscord/Discord/Objects/VoiceRegion.h
        External/QDiscord/Discord/Objects/VoiceState.cpp External/QDiscord/Discord/Objects/VoiceState.h
        External/QDiscord/Discord/Objects/Webhook.cpp External/QDiscord/Discord/Objects/Webhook.h
        External/QDiscord/Discord/Patches/ChannelPatch.cpp External/QDiscord/Discord/Patches/ChannelPatch.h
        External/QDiscord/Discord/Patches/EmojiPatch.cpp External/QDiscord/Discord/Patches/EmojiPatch.h
        External/QDiscord/Discord/Patches/GuildEmbedPatch.cpp External/QDiscord/Discord/Patches/GuildEmbedPatch.h
        External/QDiscord/Discord/Patches/GuildMemberPatch.cpp External/QDiscord/Discord/Patches/GuildMemberPatch.h
        External/QDiscord/Discord/Patches/GuildPatch.cpp External/QDiscord/Discord/Patches/GuildPatch.h
        External/QDiscord/Discord/Patches/IntegrationPatch.cpp External/QDiscord/Discord/Patches/IntegrationPatch.h
        External/QDiscord/Discord/Patches/MessagePatch.cpp External/QDiscord/Discord/Patches/MessagePatch.h
        External/QDiscord/Discord/Patches/Patch.h
        External/QDiscord/Discord/Patches/RolePatch.cpp External/QDiscord/Discord/Patches/RolePatch.h
        External/QDiscord/Discord/Patches/UserPatch.cpp External/QDiscord/Discord/Patches/UserPatch.h
        External/QDiscord/Discord/Patches/WebhookPatch.cpp External/QDiscord/Discord/Patches/WebhookPatch.h
        External/QDiscord/Discord/Payload.h External/QDiscord/Discord/Promise.h
        External/QDiscord/Discord/Token.cpp External/QDiscord/Discord/Token.h
        External/QDiscord/Discord/Url.cpp External/QDiscord/Discord/Url.h
        Helpers/audioconversionutils.cpp Helpers/audioconversionutils.h
        Helpers/audiofileholder.cpp Helpers/audiofileholder.h
        Helpers/captureholder.h Helpers/captureholder.cpp
        Helpers/jsonhelper.h Helpers/jsonhelper.cpp
        Helpers/mediadiscoverer.h Helpers/mediadiscoverer.cpp
        Helpers/ocrentrydatabase.h Helpers/ocrentrydatabase.cpp
        Helpers/peakfinder.cpp Helpers/peakfinder.h
        Helpers/serialholder.h Helpers/serialholder.cpp
        Helpers/stickpainter.h Helpers/stickpainter.cpp
        Helpers/wavfile.cpp Helpers/wavfile.h
        Managers/audiomanager.h Managers/audiomanager.cpp
        Managers/discordmanager.h Managers/discordmanager.cpp
        Managers/joystickmanager.h Managers/joystickmanager.cpp
        Managers/keyboardmanager.h Managers/keyboardmanager.cpp
        Managers/logmanager.h Managers/logmanager.cpp
        Managers/managercollection.h
        Managers/profilemanager.h Managers/profilemanager.cpp
        Managers/programmanager.h Managers/programmanager.cpp
        Managers/serialmanager.h Managers/serialmanager.cpp
        Managers/videomanager.h Managers/videomanager.cpp
        Managers/vlcmanager.h Managers/vlcmanager.cpp
        Modules/Common/framecapture.h Modules/Common/framecapture.cpp
        Modules/Common/ocr.h Modules/Common/ocr.cpp
        Modules/Common/runcommand.h Modules/Common/runcommand.cpp
        Modules/modulebase.h Modules/modulebase.cpp
        Programs/Development/devcommand.h Programs/Development/devcommand.cpp
        Programs/Development/devframecapture.h Programs/Development/devframecapture.cpp
        Programs/Development/devsounddetection.h Programs/Development/devsounddetection.cpp
        Programs/PokemonFRLG/giftreset.cpp Programs/PokemonFRLG/giftreset.h
        Programs/PokemonFRLG/nuggetfarmer.cpp Programs/PokemonFRLG/nuggetfarmer.h
        Programs/PokemonFRLG/overworldshiny.cpp Programs/PokemonFRLG/overworldshiny.h
        Programs/PokemonFRLG/permutationbase.h Programs/PokemonFRLG/permutationbase.cpp
        Programs/PokemonFRLG/starterreset.cpp Programs/PokemonFRLG/starterreset.h
        Programs/PokemonLZA/donutmaker.h Programs/PokemonLZA/donutmaker.cpp
        Programs/PokemonLZA/respawnreset.cpp Programs/PokemonLZA/respawnreset.h
        Programs/System/camerachecker.h Programs/System/camerachecker.cpp
        Programs/System/commandrecorder.h Programs/System/commandrecorder.cpp
        Programs/System/customcommand.h Programs/System/customcommand.cpp
        Programs/programbase.h Programs/programbase.cpp
        Settings/PokemonLZA/settingflavorpower.h Settings/PokemonLZA/settingflavorpower.cpp
        Settings/settingbase.h
        Settings/settingcheckbox.cpp Settings/settingcheckbox.h
        Settings/settingcolor.h Settings/settingcolor.cpp
        Settings/settingcombobox.h Settings/settingcombobox.cpp
        Settings/settingdoublespinbox.h Settings/settingdoublespinbox.cpp
        Settings/settinglanguage.h Settings/settinglanguage.cpp
        Settings/settinglineedit.h Settings/settinglineedit.cpp
        Settings/settingspinbox.h Settings/settingspinbox.cpp
        Settings/settingsystem.h Settings/settingsystem.cpp
        Settings/settingtextbrowser.h Settings/settingtextbrowser.cpp
        Settings/settingtextedit.h Settings/settingtextedit.cpp
        Settings/settingthreadpriority.h Settings/settingthreadpriority.cpp
        Types/buttontype.h
        Types/categorytype.h
        Types/languagetype.h
        Types/logtype.h
        Types/stat.h
        Types/systemtype.h
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
target_link_libraries(AutoController2 PRIVATE Qt6::Network)
target_link_libraries(AutoController2 PRIVATE Qt6::WebSockets)
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
