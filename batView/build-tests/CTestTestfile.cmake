# CMake generated Testfile for 
# Source directory: /Users/rodrigo/Documents/UCR/Proyecto_Electrico/proyecto_electrico/batView
# Build directory: /Users/rodrigo/Documents/UCR/Proyecto_Electrico/proyecto_electrico/batView/build-tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(BatViewTests "/Users/rodrigo/Documents/UCR/Proyecto_Electrico/proyecto_electrico/batView/build-tests/run_tests")
set_tests_properties(BatViewTests PROPERTIES  _BACKTRACE_TRIPLES "/Users/rodrigo/Documents/UCR/Proyecto_Electrico/proyecto_electrico/batView/CMakeLists.txt;264;add_test;/Users/rodrigo/Documents/UCR/Proyecto_Electrico/proyecto_electrico/batView/CMakeLists.txt;0;")
subdirs("external/googletest")
