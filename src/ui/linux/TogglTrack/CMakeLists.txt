set(CMAKE_CXX_FLAGS
    -Wall
)

add_definitions(-DAPP_VERSION="${TOGGL_VERSION}")
add_definitions(-DTOGGL_DATA_DIR="${TOGGL_DATA_DIR}")

# And include dirs
include_directories(
    ${TOGGLTRACKLIBRARY_INCLUDE_DIR}
    ${OAUTH_INCLUDE_DIR}
    ${BUGSNAG_INCLUDE_DIR}
    ${QXT_INCLUDE_DIRS}
)

# TogglTrack sources
set(BINARY_SOURCE_FILES
    aboutdialog.cpp
    autocompletecombobox.cpp
    autocompletelistview.cpp
    autocompletelistmodel.cpp
    autocompleteview.cpp
    clickablelabel.cpp
    colorpicker.cpp
    countryview.cpp
    errorviewcontroller.cpp
    feedbackdialog.cpp
    genericview.cpp
    idlenotificationwidget.cpp
    loginwidget.cpp
    main.cpp
    mainwindowcontroller.cpp
    networkmanagement.cpp
    overlaywidget.cpp
    powermanagement.cpp
    preferencesdialog.cpp
    settingsview.cpp
    singleapplication.cpp
    systemtray.cpp
    timeentrycellwidget.cpp
    timeentryeditorwidget.cpp
    timeentrylistwidget.cpp
    timeentryview.cpp
    timerwidget.cpp
    toggl.cpp

    # Resources have to be listed
    Resources.qrc

    # it's better to list UI files
    aboutdialog.ui
    colorpicker.ui
    errorviewcontroller.ui
    feedbackdialog.ui
    idlenotificationwidget.ui
    loginwidget.ui
    mainwindowcontroller.ui
    overlaywidget.ui
    preferencesdialog.ui
    timeentrycellwidget.ui
    timeentryeditorwidget.ui
    timeentrylistwidget.ui
    timerwidget.ui

)

# Set up compilation targets
add_executable(TogglTrack ${BINARY_SOURCE_FILES})

# And list the dependencies between them (and system libraries)
target_link_libraries(TogglTrack PRIVATE
    TogglTrackLibrary
    Qt5::Widgets Qt5::DBus Qt5::WebEngine
    OAuth2
    ${QXT_LIBRARIES}
    Bugsnag
    -lX11 -lXss
)

configure_file(${PROJECT_SOURCE_DIR}/src/ssl/cacert.pem cacert.pem COPYONLY)

configure_file(dist/com.toggl.TogglTrack.appdata.xml.in dist/com.toggl.TogglTrack.appdata.xml @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dist/com.toggl.TogglTrack.appdata.xml DESTINATION share/metainfo)

install(FILES dist/com.toggl.TogglTrack.desktop DESTINATION share/applications)

install(FILES ${PROJECT_SOURCE_DIR}/src/ssl/cacert.pem DESTINATION ${TOGGL_DATA_DIR})

install(FILES icons/16x16/toggltrack.png DESTINATION share/icons/hicolor/16x16/apps/)
install(FILES icons/24x24/toggltrack.png DESTINATION share/icons/hicolor/24x24/apps/)
install(FILES icons/32x32/toggltrack.png DESTINATION share/icons/hicolor/32x32/apps/)
install(FILES icons/48x48/toggltrack.png DESTINATION share/icons/hicolor/48x48/apps/)
install(FILES icons/64x64/toggltrack.png DESTINATION share/icons/hicolor/64x64/apps/)
install(FILES icons/96x96/toggltrack.png DESTINATION share/icons/hicolor/96x96/apps/)
install(FILES icons/128x128/toggltrack.png DESTINATION share/icons/hicolor/128x128/apps/)
install(FILES icons/256x256/toggltrack.png DESTINATION share/icons/hicolor/256x256/apps/)
if(INSTALL_HIRES_ICONS)
    install(FILES icons/1024x1024/toggltrack.png DESTINATION share/icons/hicolor/1024x1024/apps/)
endif()

install(FILES ${PROJECT_SOURCE_DIR}/src/ui/linux/TogglTrack.sh DESTINATION ${TOGGL_BINARY_DIR} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install(TARGETS TogglTrack DESTINATION ${TOGGL_BINARY_DIR})
