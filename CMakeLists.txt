cmake_minimum_required(VERSION 2.8)
project(rofi-file-browser-extended)



# Plugin

find_package(PkgConfig)
pkg_search_module(GLIB2 REQUIRED glib-2.0)
pkg_search_module(CAIRO REQUIRED cairo)
pkg_search_module(GTK3 REQUIRED gtk+-3.0)

pkg_get_variable(ROFI_PLUGINS_DIR rofi pluginsdir)

include_directories(
    include
    submodules/libnkutils/src
    ${GLIB2_INCLUDE_DIRS}
    ${CAIRO_INCLUDE_DIRS}
    ${GTK3_INCLUDE_DIRS}
)

file(GLOB SRC "src/*.c")

add_library(filebrowser SHARED ${SRC})
set_target_properties(filebrowser PROPERTIES PREFIX "")

target_link_libraries(filebrowser
    ${GLIB2_LIBRARIES}
    ${CAIRO_LIBRARIES}
    ${GTK3_LIBRARIES}
)

install(TARGETS filebrowser DESTINATION ${ROFI_PLUGINS_DIR})



# Manpage

add_custom_command(OUTPUT "doc/rofi-file-browser-extended.1.gz"
                   COMMAND gzip -k "doc/rofi-file-browser-extended.1"
                   COMMENT "Packing manpage")

add_custom_target(manpage ALL DEPENDS "doc/rofi-file-browser-extended.1.gz")

install(FILES "doc/rofi-file-browser-extended.1.gz" DESTINATION "/usr/share/man/man1")
