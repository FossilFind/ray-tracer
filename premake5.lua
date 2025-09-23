workspace "ray-tacer"
	configurations { "debug", "release" }

	filter "configurations:debug"
		defines { "DEBUG" }
		symbols "On"
	
	filter "configurations:release"
		defines { "NDEBUG" }
		optimize "On"

project "main"
	location "src"
	kind "ConsoleApp"
	language "C"
	targetdir "bin/%{cfg.buildcfg}"
	objdir "bin/%{cfg.buildcfg}/obj"

	includedirs { "src/include" }
	files { "src/**.c", "src/include/**.h" }