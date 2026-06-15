cmake_minimum_required(VERSION 3.1)
project(raudio)

add_library(raudio)
target_sources(raudio PRIVATE src/raudio.c)
target_include_directories(raudio PUBLIC include)

# raudio (raysan5/raudio master) config flags:
#  SUPPORT_MODULE_RAUDIO  - gates the whole raudio.c body (required, else it compiles to nothing)
#  RAUDIO_STANDALONE      - build without raylib (we use raudio on its own)
#  SUPPORT_FILEFORMAT_*   - only the decoders the game actually needs (WAV + FLAC)
target_compile_definitions(raudio PRIVATE
	SUPPORT_MODULE_RAUDIO
	RAUDIO_STANDALONE
	SUPPORT_FILEFORMAT_WAV
	SUPPORT_FILEFORMAT_FLAC
)

# CoreAudio backend needs Apple frameworks linked (miniaudio also runtime-links, but
# linking explicitly is the reliable path on modern macOS / Apple Silicon).
if(APPLE)
	target_link_libraries(raudio PRIVATE
		"-framework CoreAudio"
		"-framework AudioToolbox"
		"-framework AudioUnit"
		"-framework CoreFoundation"
	)
endif()