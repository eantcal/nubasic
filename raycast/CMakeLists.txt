set(NURAYCAST_ENGINE_SOURCES
  engine/ActorSystem.cpp
  engine/Player.cpp
  engine/RayCaster.cpp
  engine/RaycastEngine.cpp
  engine/SceneLoader.cpp
  engine/SpriteManager.cpp
  engine/SpriteMetadataLoader.cpp
  engine/SpriteSet.cpp
  engine/ViewWeapon.cpp
  engine/WorldJsonLoader.cpp
  engine/WorldMap.cpp)

set(NURAYCAST_ENGINE_HEADERS
  engine/ActorSystem.h
  engine/Color.h
  engine/ColumnDepthBuffer.h
  engine/FrameBuffer.h
  engine/MapCell.h
  engine/Player.h
  engine/RayCaster.h
  engine/RaycastEngine.h
  engine/SceneLoader.h
  engine/Sprite.h
  engine/SpriteManager.h
  engine/SpriteMetadataLoader.h
  engine/SpriteProjection.h
  engine/SpriteSet.h
  engine/Texture.h
  engine/ViewWeapon.h
  engine/WorldBlock.h
  engine/WorldJsonLoader.h
  engine/WorldMap.h)

add_library(nuRaycastEngine STATIC
  ${NURAYCAST_ENGINE_SOURCES}
  ${NURAYCAST_ENGINE_HEADERS})

target_include_directories(nuRaycastEngine PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/engine")

target_compile_features(nuRaycastEngine PUBLIC cxx_std_20)

target_compile_definitions(nuRaycastEngine PUBLIC
  _MBCS
  $<$<PLATFORM_ID:Windows>:WIN32>
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<NOT:$<CONFIG:Debug>>:NDEBUG>)

target_link_libraries(nuRaycastEngine PUBLIC
  nlohmann_json::nlohmann_json)

if(WIN32)
  set(NURAYCAST_WIN_SUPPORT_SOURCES
    win/BackgroundMusicPlayer.cpp
    win/D2dPresenter.cpp
    win/DdxDevice.cpp
    win/OggVorbisDecoder.cpp
    win/SoundEffectPlayer.cpp
    win/TextToSpeechPlayer.cpp
    win/WinFramePresenter.cpp
    win/WinTextureLoader.cpp)

  set(NURAYCAST_WIN_SUPPORT_HEADERS
    win/BackgroundMusicPlayer.h
    win/D2dPresenter.h
    win/DdxDevice.h
    win/OggVorbisDecoder.h
    win/SoundEffectPlayer.h
    win/TextToSpeechPlayer.h
    win/WinFramePresenter.h
    win/WinTextureLoader.h)

  add_library(nuRaycastWinSupport STATIC
    ${NURAYCAST_WIN_SUPPORT_SOURCES}
    ${NURAYCAST_WIN_SUPPORT_HEADERS})

  target_include_directories(nuRaycastWinSupport PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/win"
    "${CMAKE_CURRENT_SOURCE_DIR}/engine")

  target_compile_features(nuRaycastWinSupport PUBLIC cxx_std_20)

  target_compile_definitions(nuRaycastWinSupport PUBLIC
    WIN32
    _WINDOWS
    _MBCS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>)

  target_link_libraries(nuRaycastWinSupport PUBLIC
    nuRaycastEngine
    ddraw
    dxguid
    d2d1
    ole32
    mfplat
    mfreadwrite
    mfuuid
    winmm
    windowscodecs
    sapi)
endif()

if(MSVC)
  target_compile_options(nuRaycastEngine PRIVATE /W3)
  if(TARGET nuRaycastWinSupport)
    target_compile_options(nuRaycastWinSupport PRIVATE /W3)
  endif()
endif()
