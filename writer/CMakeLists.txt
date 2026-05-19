cmake_minimum_required(VERSION 3.21)

project(WriterQt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2 Widgets OPTIONAL_COMPONENTS SerialPort)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(WriterQt
  main.cpp
  src/app/DocumentModel.cpp
  src/app/AppSettings.cpp
  src/app/WriterController.cpp
  src/services/WriterProjectService.cpp
  src/font/FontLoader.cpp
  src/layout/LayoutEngine.cpp
  src/gcode/PathBuilder.cpp
  src/gcode/GcodeGenerator.cpp
  src/gcode/GcodeController.cpp
  src/cnc/SerialPortScan.cpp
  src/cnc/GrblConnection.cpp
  src/ui/HandwritingCanvasItem.cpp
)

# Prefer Qt SerialPort when it links; otherwise use POSIX serial on Linux.
set(_writer_serial_backend "")
if(TARGET Qt6::SerialPort)
  include(CheckCXXSourceCompiles)
  set(CMAKE_REQUIRED_LIBRARIES Qt6::SerialPort Qt6::Core)
  check_cxx_source_compiles(
    "#include <QSerialPort>\nint main(){QSerialPort p;return 0;}"
    _writer_qt_serialport_links
  )
  if(_writer_qt_serialport_links)
    set(_writer_serial_backend "qt")
    message(STATUS "WriterQt: Qt6 SerialPort enabled")
  else()
    message(WARNING
      "WriterQt: Qt6 SerialPort found but does not link with Qt6 Core ${Qt6_VERSION}. "
      "Upgrade qt6-base to match qt6-serialport (e.g. sudo pacman -Syu qt6-base qt6-serialport).")
  endif()
endif()

if(_writer_serial_backend STREQUAL "" AND UNIX AND NOT APPLE)
  set(_writer_serial_backend "posix")
  message(STATUS "WriterQt: using POSIX serial (/dev/tty*)")
endif()

if(_writer_serial_backend STREQUAL "qt")
  target_compile_definitions(WriterQt PRIVATE WRITER_HAS_SERIALPORT=1)
  target_link_libraries(WriterQt PRIVATE Qt6::SerialPort)
elseif(_writer_serial_backend STREQUAL "posix")
  target_compile_definitions(WriterQt PRIVATE WRITER_POSIX_SERIAL=1)
else()
  message(WARNING "WriterQt: no serial backend — CNC console disabled.")
endif()

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
