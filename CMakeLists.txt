# mod-city-bots - AzerothCore module build registration.
#
# REQUIRES mod-playerbots to be present and enabled. This is a playerbots
# extension, not a standalone module.

AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src")

AC_ADD_CONFIG_FILE("${CMAKE_CURRENT_LIST_DIR}/conf/mod_city_bots.conf.dist")
