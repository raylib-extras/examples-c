
baseName = path.getbasename(os.getcwd())

 workspace (baseName)
        configurations { "Debug", "Release"}
        platforms { "x64", "x86"}

        filter "configurations:Debug"
            defines { "DEBUG" }
            symbols "On"

        filter "configurations:Release"
            defines { "NDEBUG" }
            optimize "On"

        filter { "platforms:x64" }
            architecture "x86_64"

        filter {}
        
        targetdir "_bin/%{cfg.buildcfg}/"

        startproject(baseName)

    defineRaylibProject()
	
	project('lua')
    kind "StaticLib"
		location "_build"
		targetdir "_bin/%{cfg.buildcfg}"

		vpaths 
		{
			["Header Files/*"] = { "lua/src/**.h"},
			["Source Files/*"] = { "lua/src/**.c"},
		}
		files {"lua/src/*.h", "lua/src/*.c"}

    includedirs { "lua/src" }
		
    project (baseName)
        kind "ConsoleApp"
        location "_build"
        targetdir "_bin/%{cfg.buildcfg}"

        filter "action:vs*"
            debugdir "$(SolutionDir)"
		
	    filter {"action:vs*", "configurations:Release"}
            kind "WindowedApp"
            entrypoint "mainCRTStartup"
            
        filter{}

        vpaths 
        {
            ["Header Files/*"] = { "include/**.h",  "include/**.hpp", "src/**.h", "src/**.hpp", "**.h", "**.hpp"},
            ["Source Files/*"] = {"src/**.c", "src/**.cpp","**.c", "**.cpp"},
        }
        files {"*.c", "*.cpp", "*.h", "*.hpp"}

        includedirs { "./"}
		includedirs { "lua/src"}
		links('lua')
        link_raylib();