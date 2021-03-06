cmake_minimum_required(VERSION 2.8.7)
project(Cycleshooter C CXX)


set(ASSETS_DIR "${PROJECT_SOURCE_DIR}/assets")
set(DEST_DIR "${PROJECT_BINARY_DIR}/dest")
set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(VENDOR_DIR "${PROJECT_SOURCE_DIR}/vendor")
set(LIBRARY_DIR "$ENV{HOME}/.lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${DEST_DIR}/bin")
set(CYCLESHOOTER_LAUNCHER_SOURCE_DIR "${PROJECT_SOURCE_DIR}/cycleshooter-launcher")
set(CYCLESHOOTER_LAUNCHER_BUILD_DIR "${PROJECT_BINARY_DIR}/cycleshooter-launcher")


option(DEPENDENCIES_READY "Are the dependencies ready?" OFF)


add_definitions("-std=c++11
                -g
                -DLOGGING_ENABLED
                -Wno-deprecated-declarations
                -Wno-deprecated-register
                ")


add_subdirectory(${VENDOR_DIR})


if(DEPENDENCIES_READY)
    # SFML
    find_package(SFML REQUIRED EXACT 2.2 COMPONENTS audio system window)

    if(NOT SFML_FOUND)
        message(SEND_ERROR "Failed to find SFML.")
    endif()

    include_directories(${SFML_INCLUDE_DIR})


    # Ogre
    find_package(OGRE REQUIRED EXACT 1.9)

    if(NOT OGRE_FOUND)
        message(SEND_ERROR "Failed to find OGRE. Is it installed? Is it the correct version (Ogre 1.9 Ghadamon)?")
    endif()

    include_directories(
        ${OGRE_INCLUDE_DIRS}
        ${OGRE_Terrain_INCLUDE_DIRS}
        ${OGRE_Overlay_INCLUDE_DIRS}
        )

    set(
        OGRE_LIBRARIES
        ${OGRE_LIBRARIES}
        ${OGRE_Terrain_LIBRARIES}
        ${OGRE_Overlay_LIBRARIES}
        )


    # Project
    file(
        GLOB_RECURSE PROJECT_HEADERS
        "${SRC_DIR}/include/*.h"
        "${SRC_DIR}/include/*.hpp"
        )

    file(
        GLOB_RECURSE PROJECT_SOURCES
        "${SRC_DIR}/src/*.cpp"
        )

    set(PROJECT_INCLUDE_DIRS "")
    foreach(_HEADERFILE ${PROJECT_HEADERS})
        get_filename_component(_DIR ${_HEADERFILE} PATH)
        list(APPEND PROJECT_INCLUDE_DIRS ${_DIR})
    endforeach()
    list(REMOVE_DUPLICATES PROJECT_INCLUDE_DIRS)
    
    include_directories(${PROJECT_INCLUDE_DIRS})

    add_executable(
        ${PROJECT_NAME}
        ${PROJECT_HEADERS}
        ${PROJECT_SOURCES}
        )

    target_link_libraries(
        ${PROJECT_NAME}
        ${SFML_LIBRARIES}
        ${OGRE_LIBRARIES}
        )

    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target install
        )

    add_custom_target(
        launch
        DEPENDS ${PROJECT_NAME} cycleshooter-launcher
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target install
        COMMAND ./CycleshooterLauncher
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        )

    add_custom_target(
        cycleshooter-launcher
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CYCLESHOOTER_LAUNCHER_BUILD_DIR}
        COMMAND ${CMAKE_COMMAND} -E chdir ${CYCLESHOOTER_LAUNCHER_BUILD_DIR} qmake ${CYCLESHOOTER_LAUNCHER_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} -E chdir ${CYCLESHOOTER_LAUNCHER_BUILD_DIR} make
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CYCLESHOOTER_LAUNCHER_BUILD_DIR}/CycleshooterLauncher ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/CycleshooterLauncher
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CYCLESHOOTER_LAUNCHER_SOURCE_DIR}/just_do_it.mp4 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/just_do_it.mp4
        )


    file(
        GLOB_RECURSE IN_FILES
        "${ASSETS_DIR}/*.in"
        )

    # configure *.in -> *
    foreach(IN_FILE ${IN_FILES})
        string(REGEX REPLACE "\\.in$" "" OUT_FILE "${IN_FILE}")
        string(REGEX REPLACE "${ASSETS_DIR}" "${DEST_DIR}" OUT_FILE "${OUT_FILE}")
        configure_file(
            ${IN_FILE}
            ${OUT_FILE}
            @ONLY
            )
        unset(OUT_FILE)
    endforeach()

    install(
        DIRECTORY ${ASSETS_DIR}/
        DESTINATION ${DEST_DIR}
        PATTERN "*.in"
        EXCLUDE
        )
else()
    message(WARNING "*** DEPENDENCIES_READY variable is set to OFF, project won't build ***")
endif()
