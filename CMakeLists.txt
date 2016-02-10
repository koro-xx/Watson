cmake_minimum_required(VERSION 3.0)

project(Watson VERSION 0.78.0 LANGUAGES CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(Alleg REQUIRED)

if (UNIX)
	SET(LINK_LIBRARIES ${LINK_LIBRARIES} "m")
endif(UNIX)
	
if (APPLE)
	OPTION(WATSON_APP_BUNDLE "Build application bundle" ON)
	find_package(FreeType REQUIRED)
	find_package(ZLIB REQUIRED)
	find_library(OPENGL_FRAMEWORK OpenGL)
	find_library(OPENAL_FRAMEWORK OpenAL)
	find_library(IOKIT_FRAMEWORK IOKit)
	find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
	find_library(AUDIOTOOLBOX_FRAMEWORK AudioToolbox)
	find_library(APPKIT_FRAMEWORK AppKit)
	find_library(APPLICATIONSERVICES_FRAMEWORK ApplicationServices)
	# NOTE: Don't include the path in MACOSX_BUNDLE_ICON_FILE -- this is
	# the property added to Info.plist
	set(MACOSX_BUNDLE_ICON_FILE app_icon.icns)

	# And this part tells CMake where to find and install the file itself
	set(APP_ICON ${CMAKE_SOURCE_DIR}/src/app_icon.icns)
endif (APPLE)

set(SOURCES
  "src/TiledBlock.c"
  "src/allegro_stuff.c"
  "src/board.c"
  "src/game.c"
  "src/main.c"
  "src/sound.c"
  "src/bitmaps.c"
  "src/dialog.c"
  )

set(HEADERS
  "src/TiledBlock.h"
  "src/allegro_stuff.h"
  "src/board.h"
  "src/game.h"
  "src/sound.h"
  "src/bitmaps.h"
  "src/dialog.h"
)

set(RESOURCES
	"src/fonts"
	"src/icons"
	"src/sounds"
	"src/app_icon.icns"
)

add_executable(watson MACOSX_BUNDLE ${APP_ICON} ${SOURCES} ${HEADERS} ${RESOURCES})

target_link_libraries(watson
  ${LINK_LIBRARIES}
  ${APPLICATIONSERVICES_FRAMEWORK}
  ${APPKIT_FRAMEWORK}
  ${OPENAL_FRAMEWORK}
  ${IOKIT_FRAMEWORK}
  ${COREFOUNDATION_FRAMEWORK}   
  ${AUDIOTOOLBOX_FRAMEWORK}
  ${OPENGL_FRAMEWORK}
  ${ZLIB_LIBRARIES}
  ${FREETYPE_LIBRARIES}
  Alleg::Allegro
  Alleg::Audio
  Alleg::Image
  Alleg::Font
  Alleg::TTF
  Alleg::Primitives
  Alleg::Acodec
  Alleg::Color
  Alleg::Main
  )

IF(APPLE)
	SET_SOURCE_FILES_PROPERTIES(${RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
ENDIF(APPLE)
