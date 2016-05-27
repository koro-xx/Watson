cmake_minimum_required(VERSION 3.0)

project(Watson VERSION 0.78.3 LANGUAGES CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(Alleg REQUIRED)

set(LINK_LIBRARIES ${LINK_LIBRARIES}
 	  Alleg::Allegro
	  Alleg::Audio
	  Alleg::Image
	  Alleg::Font
	  Alleg::TTF
	  Alleg::Primitives
	  Alleg::Acodec
	  Alleg::Color
	  Alleg::Memfile
	  Alleg::Main
   )

if (UNIX)
	set(LINK_LIBRARIES ${LINK_LIBRARIES} "m")
endif(UNIX)

## to do: add target and copy resources postbuild for windows/linux
if (WIN32)
	set(EXECUTABLE_TYPE "WIN32")
endif(WIN32)


if (APPLE)
	option(WATSON_APP_BUNDLE "Build application bundle" ON)
	find_package(FreeType REQUIRED)
	find_package(ZLIB REQUIRED)
	find_library(OPENGL_FRAMEWORK OpenGL)
	find_library(OPENAL_FRAMEWORK OpenAL)
	find_library(IOKIT_FRAMEWORK IOKit)
	find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
	find_library(AUDIOTOOLBOX_FRAMEWORK AudioToolbox)
	find_library(APPKIT_FRAMEWORK AppKit)
	find_library(APPLICATIONSERVICES_FRAMEWORK ApplicationServices)
	set(LINK_LIBRARIES ${LINK_LIBRARIES}
  			  ${APPLICATIONSERVICES_FRAMEWORK}
			  ${APPKIT_FRAMEWORK}
			  ${OPENAL_FRAMEWORK}
			  ${IOKIT_FRAMEWORK}
			  ${COREFOUNDATION_FRAMEWORK}   
			  ${AUDIOTOOLBOX_FRAMEWORK}
			  ${OPENGL_FRAMEWORK}
			  ${ZLIB_LIBRARIES}
			  ${FREETYPE_LIBRARIES}
	   )

	# NOTE: Don't include the path in MACOSX_BUNDLE_ICON_FILE -- this is
	# the property added to Info.plist
	set(MACOSX_BUNDLE_ICON_FILE app_icon.icns)

	# And this part tells CMake where to find and install the file itself
	set(APP_ICON ${CMAKE_SOURCE_DIR}/assets/app_icon.icns)
endif (APPLE)

file(GLOB_RECURSE SOURCES "src/*.c")
file(GLOB_RECURSE HEADERS "src/*.h")

set(RESOURCES
	"assets/fonts"
	"assets/icons"
	"assets/sounds"
	"assets/data"
	"assets/buttons"
	"assets/app_icon.icns"
)

# this is for APPLE, what about WIN32 and UNIX?
add_executable(watson MACOSX_BUNDLE ${APP_ICON} ${SOURCES} ${HEADERS} ${RESOURCES})

target_link_libraries(watson ${LINK_LIBRARIES})

IF(APPLE)
	set_source_files_properties(${RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
ENDIF(APPLE)
if (WIN32)
	set_target_properties(watson PROPERTIES COMPILE_DEFINITIONS "_CRT_SECURE_NO_WARNINGS")
endif(WIN32)
