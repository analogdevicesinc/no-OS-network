no_os_sources_ifdef(CONFIG_ADIN1110 ${CMAKE_CURRENT_SOURCE_DIR}/eth_mac/lwip_eth_mac.c)
target_include_directories(no-os PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/eth_mac)
