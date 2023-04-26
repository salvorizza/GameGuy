project "GameGuy"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"include/**.h",
		"src/**.cpp"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
		"CURL_STATICLIB"
	}
	
	includedirs
	{
		"include",
		"%{wks.location}/GBZ80/include",
		"%{wks.location}/GBZ80/src",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.ImGui}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.curl}"
	}

	links
	{
		"GBZ80",
		"Glad",
		"GLFW",
		"ImGui",
		"STB",
		"Curl",
		"opengl32.lib"
	}
	
	filter "files:vendor/ImGuizmo/**.cpp"
	flags { "NoPCH" }

	filter "system:windows"
		links { "ws2_32" }
		systemversion "latest"

	filter "configurations:Debug"
		defines "GAME_GUY_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "GAME_GUY_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "GAME_GUY_DIST"
		runtime "Release"
		optimize "on"