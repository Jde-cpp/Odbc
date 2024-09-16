cmake_minimum_required( VERSION 3.20 )
set( CMAKE_MODULE_PATH $ENV{JDE_DIR}/Public/build )
include( common )
findBoost()
if( ${VCPKG} )
	SET( CMAKE_TOOLCHAIN_FILE $ENV{REPO_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake  CACHE STRING "Vcpkg toolchain file" )
endif()
#######################################################
project( Jde.DB.Odbc )
include( compiler )
#######################################################
if( MSVC )
  link_directories( $ENV{JDE_DIR}/Framework/source/.build/.bin )
	message( "link_directories( $ENV{JDE_DIR}/Framework/source/.build/.bin )" )
endif()
#######################################################
add_library( ${CMAKE_PROJECT_NAME} SHARED )
file(GLOB sources *.cpp MsSql/*.cpp )
file(GLOB headers   *.h )
target_sources( ${CMAKE_PROJECT_NAME} PRIVATE ${sources} ${headers} )

if( MSVC )
	find_package( fmt )
	target_link_libraries( ${CMAKE_PROJECT_NAME} fmt::fmt )
	target_link_libraries( ${CMAKE_PROJECT_NAME} odbc32.dll )
endif()

#######################################################
target_precompile_headers(${CMAKE_PROJECT_NAME}
	PRIVATE
	<jde/TypeDefs.h>
	"externals.h"
	<jde/log/Log.h>
	<jde/Exception.h>
	"../../Framework/source/DateTime.h"
	"TypeDefs.h"
)
if( MSVC )
	set( CMAKE_INSTALL_PREFIX $ENV{JDE_DIR}/installed )
	set( installDir ${CMAKE_INSTALL_PREFIX}/$<CONFIGURATION> )
	set( configStem jdeOdbcConfig )
	install( TARGETS ${CMAKE_PROJECT_NAME}
		EXPORT ${configStem}
		RUNTIME DESTINATION ${installDir}/bin
		ARCHIVE DESTINATION ${installDir}/lib )
	install( FILES $<TARGET_PDB_FILE:${CMAKE_PROJECT_NAME}> CONFIGURATIONS Debug RelWithDebInfo DESTINATION ${installDir}/bin )
	export( TARGETS ${CMAKE_PROJECT_NAME} NAMESPACE Jde:: FILE ${CMAKE_CURRENT_BINARY_DIR}/${configStem}.cmake )
	install( EXPORT ${configStem} DESTINATION ${CMAKE_INSTALL_PREFIX}/cmake NAMESPACE Jde:: )
endif()