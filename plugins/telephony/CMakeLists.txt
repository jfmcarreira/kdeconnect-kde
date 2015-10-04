find_package(KF5 REQUIRED COMPONENTS Notifications KCMUtils)

set(kdeconnect_telephony_SRCS
    telephonyplugin.cpp
    sendsmsdialog.cpp
)

kdeconnect_add_plugin(kdeconnect_telephony JSON kdeconnect_telephony.json SOURCES ${kdeconnect_telephony_SRCS})

target_link_libraries(kdeconnect_telephony
    kdeconnectcore
    KF5::I18n
    KF5::Notifications
)

if (TelepathyQt5_FOUND AND TelepathyQt5Service_FOUND)
target_link_libraries(kdeconnect_telephony
    connectcm
)
endif()

#######################################
# Config

set( kdeconnect_telephony_config_SRCS telephony_config.cpp )
ki18n_wrap_ui( kdeconnect_telephony_config_SRCS telephony_config.ui )

add_library(kdeconnect_telephony_config MODULE ${kdeconnect_telephony_config_SRCS} )
target_link_libraries( kdeconnect_telephony_config
    kdeconnectcore
    kdeconnectpluginkcm
    KF5::I18n
    KF5::KCMUtils
)

install( TARGETS kdeconnect_telephony_config DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kdeconnect_telephony_config.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
