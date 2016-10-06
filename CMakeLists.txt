cmake_minimum_required(VERSION 2.8)
project( itsalamp )
add_definitions(-Wall -Wextra -std=c99)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D TRACE_OFF")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -D TRACE_OFF")

find_package(GTK2 2.6 REQUIRED gtk)

if(GTK2_FOUND)
	include_directories(${GTK2_INCLUDE_DIRS})
	include_directories(include)

	add_executable( itsalamp
		src/main.c
		src/config.c
		src/image.c
		src/parse.c)
	add_executable( itsacounter
		src/countdown.c
		src/color.c
		src/parse.c)
	target_link_libraries( itsalamp ${GTK2_LIBRARIES} m)
	target_link_libraries( itsacounter m)

	install(
		TARGETS itsalamp itsacounter
		RUNTIME DESTINATION bin/
		)
	install(
		PROGRAMS tools/itsalarm tools/memon tools/cpumon tools/pacdate
		DESTINATION bin/
		)
	install(
		FILES
			icons/circle.svg
			icons/cube.svg
			icons/default.svg
			icons/diamond-1.svg
			icons/shield-1.svg
			"icons/twin moons.svg"
			icons/wallclock-t0.svg
			icons/wallclock-t1.svg
			icons/wallclock-t2.svg
			icons/wallclock-t3.svg
			icons/wallclock-t4.svg
			"icons/speed arrow-t0.svg"
			"icons/speed arrow-t1.svg"
			"icons/speed arrow-t2.svg"
			icons/trizza-t0.svg
			icons/trizza-t1.svg
			icons/trizza-t2.svg
		DESTINATION share/icons/itsalamp
		)

endif()


