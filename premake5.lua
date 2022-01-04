-- Retrieve the project name
newoption { trigger = "projectname", description = "Name of the generated project" }
local projectName = _OPTIONS["projectname"]
if projectName == nil then print("The project name was not specified! --projectname=YourApplication") end

-- Main Solution
workspace (projectName)
    configurations { "Debug", "Release" }

    platforms { "x64" }
    defaultplatform "x64"
    startproject (projectName)
    
    filter "system:not windows"
    	location "build"
    filter {}

-- Include the subprojects
include "modules/FLAME_Protocol"

-- Actual project
project (projectName)
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    location "build"
    targetdir "bin/%{cfg.buildcfg}"
    targetname (projectName)
    architecture "x86_64"

    pchheader "pch.h"
    pchsource "src/pch.cpp"

    -- Configuration filters, filters are active up to the next filter statement
    -- Indentation is purely visual

    filter "configurations:Debug"
        defines { "DEBUG", "_DEBUG", "NDEPLOY" }
        runtime "Debug"
        symbols "On"
        optimize "Off"

    filter "configurations:Release"
        defines { "NDEBUG", "NDEPLOY" }
        runtime "Release"
        symbols "On"
        optimize "On"

    filter "configurations:Deploy"
        defines { "NDEBUG", "DEPLOY" }
        runtime "Release"
        symbols "Off"
        optimize "On"

    filter {}


    -- Include directories
    local _includedirs = { 
        _SCRIPT_DIR .. "/include"
    }
    includedirs (_includedirs)

    
    -- Main source files
    files { "include/**", "src/**" }

    -- NetLib dependency
    dependson "FLAME_Protocol"
    includedirs (FLAMEPROTOCOL_INCLUDE_DIRS)
    libdirs (FLAMEPROTOCOL_LINK_DIRS)
    links (FLAMEPROTOCOL_LINKS)