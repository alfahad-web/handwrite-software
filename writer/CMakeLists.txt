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
  src/cnc/GrblConnection.cpp
  src/ui/HandwritingCanvasItem.cpp
)

set(_writer_serialport_ok FALSE)
if(TARGET Qt6::SerialPort)
  # qt6-serialport must match qt6-base (same SONAME / private API). Mismatched Arch/CachyOS
  # packages often link-fail with QIODevicePrivate undefined reference.
  set(_qt_core_ver "${Qt6_VERSION}")
  if(DEFINED Qt6SerialPort_VERSION)
    set(_qt_sp_ver "${Qt6SerialPort_VERSION}")
  else()
    set(_qt_sp_ver "${Qt6SerialPort_VERSION_STRING}")
  endif()
  if(NOT _qt_sp_ver)
    set(_qt_sp_ver "unknown")
  endif()
  if(_qt_core_ver VERSION_EQUAL _qt_sp_ver)
    set(_writer_serialport_ok TRUE)
    message(STATUS "WriterQt: Qt6 SerialPort enabled (${_qt_sp_ver})")
  else()
    message(WARNING
      "WriterQt: Qt6 SerialPort ${_qt_sp_ver} does not match Qt6 Core ${_qt_core_ver} — "
      "CNC serial disabled (stub). Upgrade qt6-base to match qt6-serialport, e.g.:\n"
      "  sudo pacman -S qt6-base qt6-serialport")
  endif()
  unset(_qt_core_ver)
  unset(_qt_sp_ver)
elseif(Qt6SerialPort_FOUND)
  message(WARNING "WriterQt: Qt6::SerialPort target missing.")
else()
  message(WARNING "WriterQt: Qt6 SerialPort not found — CNC console stubbed. Install qt6-serialport.")
endif()

if(_writer_serialport_ok)
  target_compile_definitions(WriterQt PRIVATE WRITER_HAS_SERIALPORT=1)
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

if(_writer_serialport_ok)
  target_link_libraries(WriterQt PRIVATE Qt6::SerialPort)
endif()
unset(_writer_serialport_ok)
