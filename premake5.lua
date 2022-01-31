newoption 
{
   trigger = "opengl43",
   description = "use OpenGL 4.3"
}

function platform_defines()
	defines{"PLATFORM_DESKTOP"}
	if (_OPTIONS["opengl43"]) then
		defines{"GRAPHICS_API_OPENGL_43"}
	else
		defines{"GRAPHICS_API_OPENGL_33"}
	end
end

workspace "Examples-c"
	configurations { "Debug","Debug.DLL", "Release", "Release.DLL" }
	platforms { "x64", "x86"}

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
		
	filter "configurations:Debug.DLL"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"	
		
	filter "configurations:Release.DLL"
		defines { "NDEBUG" }
		optimize "On"	
		
	filter { "platforms:x64" }
		architecture "x86_64"
		
	targetdir "bin/%{cfg.buildcfg}/"
	
project "raylib"
	filter "configurations:Debug.DLL OR Release.DLL"
		kind "SharedLib"
		defines {"BUILD_LIBTYPE_SHARED"}
		
	filter "configurations:Debug OR Release"
		kind "StaticLib"
		
	filter "system:windows"
		defines{"_WINSOCK_DEPRECATED_NO_WARNINGS", "_CRT_SECURE_NO_WARNINGS", "_WIN32"}
		links {"winmm", "kernel32", "opengl32", "kernel32", "gdi32"}
		
	filter "system:linux"
		links {"pthread", "GL", "m", "dl", "rt", "X11"}
		
	filter{}
	platform_defines()

	location "build"
	language "C"
	targetdir "bin/%{cfg.buildcfg}"
	
	includedirs { "raylib/src", "raylib/src/external/glfw/include"}
	vpaths 
	{
		["Header Files"] = { "raylib/src/**.h"},
		["Source Files/*"] = {"raylib/src/**.c"},
	}
	files {"raylib/src/*.h", "raylib/src/*.c"}
		
project "TextureRepeat"
	kind "ConsoleApp"
	location "texture_repeat"
	language "C"
	targetdir "bin/%{cfg.buildcfg}"
	
	vpaths 
	{
		["Header Files"] = { "**.h"},
		["Source Files"] = {"**.c", "**.cpp"},
	}
	files {"texture_repeat/**.c", "texture_repeat/**.h"}

	links {"raylib"}
	
	includedirs { "texture_repeat", "raylib/src" }
	platform_defines()
	
	filter "action:vs*"
		defines{"_WINSOCK_DEPRECATED_NO_WARNINGS", "_CRT_SECURE_NO_WARNINGS"}
		dependson {"raylib"}
		links {"raylib.lib"}
        characterset ("MBCS")
		
	filter "system:windows"
		defines{"_WIN32"}
		links {"winmm", "kernel32", "opengl32", "kernel32", "gdi32"}
		libdirs {"bin/%{cfg.buildcfg}"}
		
	filter "system:linux"
		links {"pthread", "GL", "m", "dl", "rt", "X11"}
		
project "CircleInView2d"
	kind "ConsoleApp"
	location "circle_in_view_2d"
	language "C"
	targetdir "bin/%{cfg.buildcfg}"
	
	vpaths 
	{
		["Header Files"] = { "**.h"},
		["Source Files"] = {"**.c", "**.cpp"},
	}
	files {"circle_in_view_2d/**.c", "circle_in_view_2d/**.h"}

	links {"raylib"}
	
	includedirs { "circle_in_view_2d", "raylib/src" }
	platform_defines()
		
	filter "action:vs*"
		defines{"_WINSOCK_DEPRECATED_NO_WARNINGS", "_CRT_SECURE_NO_WARNINGS"}
		dependson {"raylib"}
		links {"raylib.lib"}
        characterset ("MBCS")
		
	filter "system:windows"
		defines{"_WIN32"}
		links {"winmm", "kernel32", "opengl32", "kernel32", "gdi32"}
		libdirs {"bin/%{cfg.buildcfg}"}
		
	filter "system:linux"
		links {"pthread", "GL", "m", "dl", "rt", "X11"}
		
project "ray2d_rect_intersection"
	kind "ConsoleApp"
	location "ray2d_rect_intersection"
	language "C"
	targetdir "bin/%{cfg.buildcfg}"
	
	vpaths 
	{
		["Header Files"] = { "**.h"},
		["Source Files"] = {"**.c", "**.cpp"},
	}
	files {"ray2d_rect_intersection/**.c", "ray2d_rect_intersection/**.h"}

	links {"raylib"}
	
	includedirs { "ray2d_rect_intersection", "raylib/src" }
	platform_defines()
		
	filter "action:vs*"
		defines{"_WINSOCK_DEPRECATED_NO_WARNINGS", "_CRT_SECURE_NO_WARNINGS"}
		dependson {"raylib"}
		links {"raylib.lib"}
        characterset ("MBCS")
		
	filter "system:windows"
		defines{"_WIN32"}
		links {"winmm", "kernel32", "opengl32", "kernel32", "gdi32"}
		libdirs {"bin/%{cfg.buildcfg}"}
		
	filter "system:linux"
		links {"pthread", "GL", "m", "dl", "rt", "X11"}
		
project "UnsortedBilboards"
	kind "ConsoleApp"
	location "unsorted_bilboards"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"
	cppdialect "C++17"
	
	vpaths 
	{
		["Header Files"] = { "**.h"},
		["Source Files"] = {"**.c", "**.cpp"},
	}
	files {"unsorted_bilboards/**.c", "unsorted_bilboards/**.cpp", "unsorted_bilboards/**.h"}

	links {"raylib"}
	
	includedirs { "unsorted_bilboards", "raylib/src" }
	platform_defines()
	
	filter "action:vs*"
		defines{"_WINSOCK_DEPRECATED_NO_WARNINGS", "_CRT_SECURE_NO_WARNINGS"}
		dependson {"raylib"}
		links {"raylib.lib"}
        characterset ("MBCS")
		
	filter "system:windows"
		defines{"_WIN32"}
		links {"winmm", "kernel32", "opengl32", "kernel32", "gdi32"}
		libdirs {"bin/%{cfg.buildcfg}"}
		
	filter "system:linux"
		links {"pthread", "GL", "m", "dl", "rt", "X11"}
		
project "RectCircleCollisions"
	kind "ConsoleApp"
	location "rect_circle_collisions"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"
	cppdialect "C++17"
	
	vpaths 
	{
		["Header Files"] = { "**.h"},
		["Source Files"] = {"**.c", "**.cpp"},
	}
	files {"rect_circle_collisions/**.c", "rect_circle_collisions/**.cpp", "rect_circle_collisions/**.h"}

	links {"raylib"}
	
	includedirs { "rect_circle_collisions", "raylib/src" }
	platform_defines()
	
	filter "action:vs*"
		defines{"_WINSOCK_DEPRECATED_NO_WARNINGS", "_CRT_SECURE_NO_WARNINGS"}
		dependson {"raylib"}
		links {"raylib.lib"}
        characterset ("MBCS")
		
	filter "system:windows"
		defines{"_WIN32"}
		links {"winmm", "kernel32", "opengl32", "kernel32", "gdi32"}
		libdirs {"bin/%{cfg.buildcfg}"}
		
	filter "system:linux"
		links {"pthread", "GL", "m", "dl", "rt", "X11"}