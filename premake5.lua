-- require "cmake"

workspace "Opium"
    architecture "x64"
    startproject "OpiumApp"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder ( solution directory )
IncludeDir = {}
IncludeDir["GLFW"] = "Opium/external/GLFW/include"
IncludeDir["Glad"] = "Opium/external/Glad/include"
IncludeDir["ImGui"] = "Opium/external/imgui"
IncludeDir["glm"] = "Opium/external/glm"
IncludeDir["stb_image"] = "Opium/external/stb_image"
IncludeDir["entt"] = "Opium/external/entt/include"
IncludeDir["yaml_cpp"] = "Opium/external/yaml-cpp/include"


include "Opium/external/GLFW"
include "Opium/external/Glad"
include "Opium/external/imgui"
include "Opium/external/yaml-cpp"

project "Opium"
    location "Opium"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "Precomp.h"
    pchsource "Opium/src/Config/Precomp.cpp"

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/external/stb_image/**.h",
        "%{prj.name}/external/stb_image/**.cpp",
        "%{prj.name}/external/glm/glm/**.hpp",
        "%{prj.name}/external/glm/glm/**.inl",
        "%{prj.name}/external/entt/include/**.hpp"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
	"YAML_CPP_STATIC_DEFINE"
    }

    includedirs
    {
        "%{prj.name}/src",
	    "%{prj.name}/src/Config",
        "%{prj.name}/external/spdlog/include",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.yaml_cpp}"
    }

    links
    {
        "GLFW",
        "Glad",
        "ImGui",
        "yaml-cpp",
        "opengl32.lib"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "OP_PLATFORM_WINDOWS",
            "OP_BUILD_DLL",
            "GLFW_INCLUDE_NONE"
        }

    filter "configurations:Debug"
        defines "OP_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "OP_RELEASE"
        runtime "Release"
        optimize "speed"

    filter "configurations:Dist"
        defines "OP_DIST"
        runtime "Release"
        optimize "on"


project "OpiumApp"
    location "Apps/OpiumApp"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "Apps/%{prj.name}/src/**.h",
        "Apps/%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "Opium/external/spdlog/include",
        "Opium/src",
	    "Opium/src/Config",
        "Opium/external",
        "%{IncludeDir.glm}",
	    "Apps/%{prj.name}/src"
    }

    links
    {
        "Opium"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "OP_PLATFORM_WINDOWS"
        }

    filter "configurations:Debug"
        defines "OP_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "OP_RELEASE"
        runtime "Release"
        optimize "speed"

    filter "configurations:Dist"
        defines "OP_DIST"
        runtime "Release"
        optimize "on"
    
    
project "Opium_Editor"
    location "Apps/Opium_Editor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "Apps/%{prj.name}/src/**.h",
        "Apps/%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "Opium/external/spdlog/include",
        "Opium/src",
	    "Opium/src/Config",
        "Opium/external",
        "%{IncludeDir.glm}",
        "%{IncludeDir.entt}",
	    "Apps/%{prj.name}/src"
    }

    links
    {
        "Opium"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "OP_PLATFORM_WINDOWS"
        }

    filter "configurations:Debug"
        defines "OP_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "OP_RELEASE"
        runtime "Release"
        optimize "speed"

    filter "configurations:Dist"
        defines "OP_DIST"
        runtime "Release"
        optimize "on"
    
    

    

        