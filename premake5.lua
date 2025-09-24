os.execute("cmake -S lib/glfw -B lib/glfw/build")
os.execute("cmake --build lib/glfw/build")

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

		links { "lib/glfw/build/src/glfw3", "m" }

		includedirs { "src/include", "lib/glad/include", "lib/glfw/include" }
		files { "src/**.c", "src/include/**.h", "lib/glad/src/**.c", "lib/glad/include/**.h" }

		filter "system:linux"
			links { "GL" }

		filter "system:windows"
			links { "opengl" }