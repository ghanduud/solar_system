workspace "sfml_tasks"
	configurations { "Debug", "Release" }
	location "build"
    architecture"X32"
	
	project "sfml_tasks"
	kind "ConsoleApp"
	language "C++"
    cppdialect "C++17"
	targetdir "bin/%{cfg.buildcfg}"
	
	defines { "SFML_STATIC" }
	files { "src/**.*" }

	libdirs { "dependencies/SFML/lib" }
	includedirs { "dependencies/SFML/include" }

	libdirs { "dependencies/Box2D/bin/debug" }
	includedirs { "dependencies/Box2D/include/box2d" }

	filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        links {
            "sfml-graphics-s-d",
            "sfml-window-s-d",
            "sfml-system-s-d",
            "opengl32",  -- Additional dependencies for SFML
            "freetype",  -- Add other dependencies as necessary
            "winmm",     -- For Windows multimedia API
            "gdi32",      -- Windows GDI
			"openal32",
			"flac",
			"vorbisenc",
			"vorbisfile",
			"vorbis",
			"ogg",
			"sfml-audio-s-d",
			"box2d"
        }
		
		filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        links {
			"sfml-graphics-s",
            "sfml-window-s",
            "sfml-system-s",
            "opengl32",  -- Additional dependencies for SFML
            "freetype",  -- Add other dependencies as necessary
            "winmm",     -- For Windows multimedia API
            "gdi32",      -- Windows GDI
			"openal32",
			"flac",
			"vorbisenc",
			"vorbisfile",
			"vorbis",
			"ogg",
			"sfml-audio-s",
			"box2d"
        }
