cmake_minimum_required(VERSION 3.21)

project(HandwriteQt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 Widgets)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(HandwriteQt
  main.cpp
  src/core/EditorStore.cpp
  src/core/EditorStore.h
  src/canvas/CanvasItem.cpp
  src/canvas/CanvasItem.h
  src/services/ExportService.cpp
  src/services/ExportService.h
  src/services/FileService.cpp
  src/services/FileService.h
  src/services/ProjectService.cpp
  src/services/ProjectService.h
  src/app/AppController.cpp
  src/app/AppController.h
)

qt_add_resources(HandwriteQt "qml_resources"
  PREFIX "/"
  FILES
    qml/Main.qml
)

target_link_libraries(HandwriteQt
  PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Widgets
)
