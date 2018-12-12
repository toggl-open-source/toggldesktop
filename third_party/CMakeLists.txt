
set(OAUTH_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/qt-oauth-lib
    PARENT_SCOPE
)
set(BUGSNAG_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/bugsnag-qt
    PARENT_SCOPE
)

# And include dirs
include_directories(
    qt-oauth-lib
    bugsnag-qt
)

# OAuth has to be bundled
set(OAUTH2_SOURCE_FILES
    qt-oauth-lib/logindialog.cpp
    qt-oauth-lib/oauth2.cpp
)

# Bugsnag has to be bundled
set(BUGSNAG_SOURCE_FILES
    bugsnag-qt/bugsnag.cpp
)

# Set up compilation targets
add_library(OAuth2 SHARED ${OAUTH2_SOURCE_FILES})
add_library(Bugsnag SHARED ${BUGSNAG_SOURCE_FILES})

# And list the dependencies between them (and system libraries)a
target_link_libraries(OAuth2 PRIVATE
    Qt5::Widgets Qt5::WebEngine Qt5::WebEngineWidgets
)
target_link_libraries(Bugsnag PRIVATE
    Qt5::Widgets Qt5::Network
)
