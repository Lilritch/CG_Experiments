cmake_minimum_required(VERSION 3.10)
project(bird_scene LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(bird_scene
    main.cpp
    OBJImporter.cpp
)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

if(TARGET OpenGL::GL)
    target_link_libraries(bird_scene PRIVATE OpenGL::GL)
elseif(OPENGL_LIBRARIES)
    target_link_libraries(bird_scene PRIVATE ${OPENGL_LIBRARIES})
endif()

if(TARGET GLUT::GLUT)
    target_link_libraries(bird_scene PRIVATE GLUT::GLUT)
elseif(GLUT_LIBRARY)
    target_link_libraries(bird_scene PRIVATE ${GLUT_LIBRARY})
elseif(GLUT_LIBRARIES)
    target_link_libraries(bird_scene PRIVATE ${GLUT_LIBRARIES})
endif()

if(GLUT_INCLUDE_DIRS)
    target_include_directories(bird_scene PRIVATE ${GLUT_INCLUDE_DIRS})
endif()

# Windows-specific FreeGLUT fallback for environments where FindGLUT
# locates headers but does not expose the runtime import library cleanly.
if(WIN32)
    find_library(FREEGLUT_LIBRARY NAMES freeglut libfreeglut)
    if(FREEGLUT_LIBRARY)
        message(STATUS "Found FreeGLUT fallback library: ${FREEGLUT_LIBRARY}")
        target_link_libraries(bird_scene PRIVATE ${FREEGLUT_LIBRARY})
    endif()
endif()
