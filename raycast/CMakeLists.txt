set(NURCADE_ENGINE_SOURCES
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

set(NURCADE_ENGINE_HEADERS
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

add_library(nuRCADEEngine STATIC
  ${NURCADE_ENGINE_SOURCES}
  ${NURCADE_ENGINE_HEADERS})

target_include_directories(nuRCADEEngine PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/engine")

target_compile_features(nuRCADEEngine PUBLIC cxx_std_20)

target_compile_definitions(nuRCADEEngine PUBLIC
  _MBCS
  $<$<PLATFORM_ID:Windows>:WIN32>
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<NOT:$<CONFIG:Debug>>:NDEBUG>)

target_link_libraries(nuRCADEEngine PUBLIC
  nlohmann_json::nlohmann_json)

if(WIN32)
  add_library(nuRCADETextureLoader STATIC
    win/WinTextureLoader.cpp
    win/WinTextureLoader.h)

  target_include_directories(nuRCADETextureLoader PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/win"
    "${CMAKE_CURRENT_SOURCE_DIR}/engine")

  target_compile_features(nuRCADETextureLoader PUBLIC cxx_std_20)

  target_compile_definitions(nuRCADETextureLoader PUBLIC
    WIN32
    _WINDOWS
    _MBCS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>)

  target_link_libraries(nuRCADETextureLoader PUBLIC
    nuRCADEEngine
    ole32
    windowscodecs)

  set(NURCADE_WIN_SUPPORT_SOURCES
    win/BackgroundMusicPlayer.cpp
    win/D2dPresenter.cpp
    win/DdxDevice.cpp
    win/OggVorbisDecoder.cpp
    win/SoundEffectPlayer.cpp
    win/TextToSpeechPlayer.cpp
    win/WinFramePresenter.cpp
    win/WinTextureLoader.cpp)

  set(NURCADE_WIN_SUPPORT_HEADERS
    win/BackgroundMusicPlayer.h
    win/D2dPresenter.h
    win/DdxDevice.h
    win/OggVorbisDecoder.h
    win/SoundEffectPlayer.h
    win/TextToSpeechPlayer.h
    win/WinFramePresenter.h
    win/WinTextureLoader.h)

  add_library(nuRCADEWinSupport STATIC
    ${NURCADE_WIN_SUPPORT_SOURCES}
    ${NURCADE_WIN_SUPPORT_HEADERS})

  target_include_directories(nuRCADEWinSupport PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/win"
    "${CMAKE_CURRENT_SOURCE_DIR}/engine")

  target_compile_features(nuRCADEWinSupport PUBLIC cxx_std_20)

  target_compile_definitions(nuRCADEWinSupport PUBLIC
    WIN32
    _WINDOWS
    _MBCS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>)

  target_link_libraries(nuRCADEWinSupport PUBLIC
    nuRCADEEngine
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
  target_compile_options(nuRCADEEngine PRIVATE /W3)
  if(TARGET nuRCADETextureLoader)
    target_compile_options(nuRCADETextureLoader PRIVATE /W3)
  endif()
  if(TARGET nuRCADEWinSupport)
    target_compile_options(nuRCADEWinSupport PRIVATE /W3)
  endif()
endif()

if(BUILD_TESTING)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  set(INSTALL_GMOCK OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  FetchContent_MakeAvailable(googletest)

  add_executable(nuRCADEEngineTests
    tests/ActorSystemTests.cpp
    tests/ColumnDepthBufferTests.cpp
    tests/FrameBufferTests.cpp
    tests/MapCellTests.cpp
    tests/RayCasterTests.cpp
    tests/RaycastEngineSpriteTests.cpp
    tests/RenderRegressionTests.cpp
    tests/SceneLoaderTests.cpp
    tests/SpriteMetadataLoaderTests.cpp
    tests/SpriteProjectionTests.cpp
    tests/SpriteSetTests.cpp
    tests/SpriteTests.cpp
    tests/TextureTests.cpp
    tests/ViewWeaponTests.cpp
    tests/WorldJsonLoaderTests.cpp
    tests/BenchmarkScene.h)

  target_include_directories(nuRCADEEngineTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/engine")

  target_compile_definitions(nuRCADEEngineTests PRIVATE
    NURCADE_DEMO_WORLD_DIR="${CMAKE_SOURCE_DIR}/examples/raycast/raycast_demo/worlds")

  target_link_libraries(nuRCADEEngineTests PRIVATE
    nuRCADEEngine
    GTest::gtest_main)

  include(GoogleTest)
  gtest_discover_tests(nuRCADEEngineTests
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    DISCOVERY_MODE PRE_TEST)

  if(MSVC)
    target_compile_options(nuRCADEEngineTests PRIVATE /W3)
    set_target_properties(nuRCADEEngineTests PROPERTIES FOLDER "Tests")
  endif()
endif()
