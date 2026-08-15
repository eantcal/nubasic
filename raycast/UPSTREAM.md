# WinRayCast upstream sync

nuBASIC embeds the WinRayCast engine so BASIC programs can use the renderer
through the `Ray*` runtime API.

Current upstream reference:

- Repository: `C:\repo\winraycast`
- Commit: `e7aec8272e71b07bdae27baf98f6b1338c922dc1`
- Commit summary: `Fix sprite corner collision near doors`

Path mapping:

- `winraycast/src/engine` -> `nubasic/raycast/engine`
- `winraycast/src/win` -> `nubasic/raycast/win`
- `winraycast/tests` -> `nubasic/raycast/tests`
- `winraycast/scene_loader_fixtures` -> `nubasic/raycast/scene_loader_fixtures`
- `winraycast/sprite_metadata_loader_fixtures` -> `nubasic/raycast/sprite_metadata_loader_fixtures`
- `winraycast/res/worlds/demo_embedded` -> `nubasic/examples/raycast/raycast_demo/worlds`
- `winraycast/third_party/stb/stb_vorbis.c` -> `nubasic/third_party/stb/stb_vorbis.c`

Sync notes:

- Engine sources are expected to stay byte-for-byte close to upstream, apart
  from line ending and local build integration differences.
- Windows support sources may carry nuBASIC-specific optional dependency
  guards, especially around Ogg Vorbis and installer-friendly builds.
- Demo resource backup files from upstream are intentionally not imported.
