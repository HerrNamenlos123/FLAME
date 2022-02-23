
-- Utility functions
function appendTable(tableA, tableB)
    for _,v in ipairs(tableB) do 
        table.insert(tableA, v) 
    end
end

NETLIB_WANT_SPDLOG = true   -- We want to re-use the NetLib's spdlog module

-- Include the subprojects
include "modules/FLAME_Protocol"
include "modules/NetLib"

-- Main library project
project "FLAME"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    location "build"
    targetname "FLAME"
    targetdir "bin/%{cfg.buildcfg}"
    --system "Windows"
    --architecture "x86_64"

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
    files ({ "include/**", "src/**" })

    
    -- FLAME_Protocol dependency
    dependson "FLAME_Protocol"
    includedirs (FLAMEPROTOCOL_INCLUDE_DIRS)
    libdirs (FLAMEPROTOCOL_LINK_DIRS)
    links (FLAMEPROTOCOL_LINKS)

    -- NetLib dependency
    dependson "NetLib"
    includedirs (NETLIB_INCLUDE_DIRS)
    libdirs (NETLIB_LINK_DIRS)
    links (NETLIB_LINKS)
    defines (NETLIB_DEFINES)





    -- Include and linker information for premake projects using this library
    FLAME_INCLUDE_DIRS = {}
    appendTable(FLAME_INCLUDE_DIRS, _includedirs)

    FLAME_LINK_DIRS = {}
    appendTable(FLAME_LINK_DIRS, _SCRIPT_DIR .. "/bin/%{cfg.buildcfg}/")
    appendTable(FLAME_LINK_DIRS, FLAMEPROTOCOL_LINK_DIRS)
    appendTable(FLAME_LINK_DIRS, NETLIB_LINK_DIRS)

    FLAME_LINKS = { "FLAME" }
    appendTable(FLAME_LINKS, FLAMEPROTOCOL_LINKS)
    appendTable(FLAME_LINKS, NETLIB_LINKS)

    FLAME_DEFINES = {}
    appendTable(FLAME_DEFINES, NETLIB_DEFINES)
