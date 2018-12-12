# Set up some compilation definitions
add_definitions(
    -std=c++14
    -O2
    -Wall
    -Wextra
    -DAPP_ENVIRONMENT="CMAKE"
    -DAPP_VERSION="0"
)

# And include dirs
include_directories(
    ${TOGGLDESKTOPLIBRARY_INCLUDE_DIR}
    ${OAUTH_INCLUDE_DIR}
    ${BUGSNAG_INCLUDE_DIR}
)

# TogglDesktop sources
set(BINARY_SOURCE_FILES
    aboutdialog.cpp
    autocompleteview.cpp
    clickablelabel.cpp
    colorpicker.cpp
    countryview.cpp
    errorviewcontroller.cpp
    feedbackdialog.cpp
    genericview.cpp
    idlenotificationdialog.cpp
    loginwidget.cpp
    main.cpp
    mainwindowcontroller.cpp
    overlaywidget.cpp
    preferencesdialog.cpp
    settingsview.cpp
    singleapplication.cpp
    timeentrycellwidget.cpp
    timeentryeditorwidget.cpp
    timeentrylistwidget.cpp
    timeentryview.cpp
    timerwidget.cpp
    toggl.cpp

    # Resources have to be listed, .ui files don't
    Resources.qrc
)

# Set up compilation targets
add_executable(TogglDesktop ${BINARY_SOURCE_FILES})

# And list the dependencies between them (and system libraries)
target_link_libraries(TogglDesktop PRIVATE
    TogglDesktopLibrary
    Qt5::Widgets Qt5::WebEngine
    OAuth2
    ${QXT_CORE_LIBRARIES} ${QXT_WIDGETS_LIBRARIES}
    Bugsnag 
    #Poco::Crypto Poco::DataSQLite Poco::NetSSL
    PocoCrypto PocoDataSQLite PocoNetSSL
    -lX11 -lXss
)
