#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Octree SceneManager build

set (HEADER_FILES
  include/OgreHeightmapTerrainPageSource.h
  include/OgreOctree.h
  include/OgreOctreeCamera.h
  include/OgreOctreeNode.h
  include/OgreOctreePlugin.h
  include/OgreOctreeSceneManager.h
  include/OgreOctreeSceneQuery.h
  include/OgreTerrainPage.h
  include/OgreTerrainPageSource.h
  include/OgreTerrainPrerequisites.h
  include/OgreTerrainRenderable.h
  include/OgreTerrainSceneManager.h
  include/OgreTerrainVertexProgram.h
)

set (SOURCE_FILES
  src/OgreHeightmapTerrainPageSource.cpp
  src/OgreOctree.cpp
  src/OgreOctreeCamera.cpp
  src/OgreOctreeNode.cpp
  src/OgreOctreePlugin.cpp
  src/OgreOctreeSceneManager.cpp
  src/OgreOctreeSceneManagerDll.cpp
  src/OgreOctreeSceneQuery.cpp
  src/OgreTerrainPage.cpp
  src/OgreTerrainPageSource.cpp
  src/OgreTerrainRenderable.cpp
  src/OgreTerrainSceneManager.cpp
  src/OgreTerrainVertexProgram.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_definitions(-D_USRDLL)

add_library(Plugin_OctreeSceneManager ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(Plugin_OctreeSceneManager OgreMain)
if (NOT OGRE_STATIC)
  set_target_properties(Plugin_OctreeSceneManager PROPERTIES
    COMPILE_DEFINITIONS OGRE_TERRAINPLUGIN_EXPORTS
  ) 
endif ()

ogre_config_plugin(Plugin_OctreeSceneManager)
if (OGRE_INSTALL_PLUGINS_HEADERS)
  install(FILES ${HEADER_FILES} DESTINATION include/OGRE/Plugins/OctreeSceneManager)
endif ()

