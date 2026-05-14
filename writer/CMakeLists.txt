cmake_minimum_required(VERSION 3.21)

project(WriterQt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 Widgets)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(WriterQt
  main.cpp
  src/app/DocumentModel.cpp
  src/app/AppSettings.cpp
  src/app/WriterController.cpp
  src/services/WriterProjectService.cpp
  src/font/FontLoader.cpp
  src/layout/LayoutEngine.cpp
  src/ui/HandwritingCanvasItem.cpp
)

qt_add_resources(WriterQt "qml_resources"
  PREFIX "/"
  FILES
    qml/Main.qml
)

target_include_directories(WriterQt PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(WriterQt
  PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Widgets
)
