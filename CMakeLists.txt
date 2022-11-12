cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_STANDARD 17)

project(
	iwmtool
	VERSION 0.0.1)

include(FeatureSummary)
include(GNUInstallDirs)

option(SYSTEM_CXXOPTS "Use the system provided cxxopts" OFF)

if(SYSTEM_CXXOPTS)
	find_package(cxxopts REQUIRED)
endif()

add_subdirectory(iwmtool)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
