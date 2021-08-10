cmake_minimum_required(VERSION 3.14)

project(us3dViewer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND QML_IMPORT_PATH "/usr/local/lib/qml")
list(APPEND VTK_QML_DIR ${QML_IMPORT_PATH})
message(INFO ">>>QML_IMPORT_PATH=${QML_IMPORT_PATH}")
set(ENV{QML2_IMPORT_PATH} ${QML_IMPORT_PATH})
message(INFO ">>>ENV{QML2_IMPORT_PATH}=$ENV{QML2_IMPORT_PATH}")
#set(QML2_IMPORT_PATH ${QML_IMPORT_PATH}
#    CACHE STRING "Qt Creator 4.1 extra qml import paths"
#    FORCE
#)
if(NOT DEFINED ENV{QML2_IMPORT_PATH})
    message(WARNING ">>>not defined environment variable:${QML2_IMPORT_PATH}")
endif()
# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check https://doc.qt.io/qt/deployment-android.html for more information.
# They need to be set before the find_package(...) calls below.

find_package(VTK
  COMPONENTS
    CommonCore
    GUISupportQt
    GUISupportQtQuick
    IOImage
    ImagingColor
    ImagingGeneral
    InteractionImage
    InteractionStyle
    InteractionWidgets
    REQUIRED)
if (NOT VTK_FOUND)
	message(ERROR ">>>Skipping example: ${VTK_NOT_FOUND_MESSAGE}")
  return ()
endif ()

message(INFO ">>>VTK PACKAGES_FOUND")

find_package(QT NAMES Qt5 COMPONENTS Core Quick LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick LinguistTools REQUIRED)


set(PROJECT_SOURCES
        main.cpp
        qml.qrc
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_compile_definitions(${PROJECT_NAME}
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(${PROJECT_NAME}
  PRIVATE ${VTK_LIBRARIES} Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Quick )


vtk_module_autoinit(
  TARGETS ${PROJECT_NAME}
  MODULES ${VTK_LIBRARIES})

#below cmd can not work to replace engine.addImportPath()
set_property(
  TARGET ${PROJECT_NAME}
  APPEND
  PROPERTY
    ENVIRONMENT "QML2_IMPORT_PATH=${VTK_QML_DIR}")
