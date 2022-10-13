add_executable(iwmtool
	iw3/iwm_encdec.c
	iw3/iwm_encdec.h
	iw3/md4.c
	iw3/md4.h
	iwmtool.cpp
	iwm.cpp
	iwm.h)

if(NOT SYSTEM_CXXOPTS)
	target_include_directories(iwmtool PUBLIC ${CMAKE_SOURCE_DIR}/vendor/cxxopts/include)
endif()

install(TARGETS iwmtool DESTINATION ${CMAKE_INSTALL_BINDIR})
