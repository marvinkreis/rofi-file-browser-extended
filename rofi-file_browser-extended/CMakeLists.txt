cmake_minimum_required(VERSION 2.8)
project(rofi-file_browser-extended)

find_package(PkgConfig)
pkg_search_module(GLIB2 REQUIRED glib-2.0)

pkg_search_module(CAIRO REQUIRED cairo)

include_directories(
	../submodules/libnkutils/src
	${GLIB2_INCLUDE_DIRS}
	${CAIRO_INCLUDE_DIRS}
)

add_library(file_browser SHARED src/filebrowser.c)
set_target_properties(file_browser PROPERTIES PREFIX "")

target_link_libraries(file_browser
	${GLIB2_LIBRARIES}
	${CAIRO_LIBRARIES}
)
