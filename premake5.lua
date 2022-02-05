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

function define_project(project_folder)
	kind "ConsoleApp"
	location(project_folder)
	language "C"
	targetdir "bin/%{cfg.buildcfg}"
	
	vpaths 
	{
		["Header Files"] = { "**.h"},
		["Source Files"] = {"**.c"},
	}
	files {project_folder.."/**.c", project_folder.."/**.h"}

	links {"raylib"}
	
	includedirs { project_folder, "raylib/src" }
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
	define_project("texture_repeat")

project "CircleInView2d"
	define_project("circle_in_view_2d")
	
project "Ray2dRectIntersection"
	define_project("ray2d_rect_intersection")
		
project "UnsortedBilboards"
	define_project("unsorted_bilboards")
		
project "RectCircleCollisions"
	define_project("rect_circle_collisions")
