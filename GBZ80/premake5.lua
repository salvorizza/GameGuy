project "GBZ80"
	kind "StaticLib"
	language "C"
	cdialect "C11"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	--pchheader "hzpch.h"
	--pchsource "src/hzpch.cpp"

	files
	{
		"include/**.h",
		"src/**.c"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs
	{
		"include"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
		}

	filter "configurations:Debug"
		defines "GBZ80_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "GBZ80_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "GBZ80_DIST"
		runtime "Release"
		optimize "on"