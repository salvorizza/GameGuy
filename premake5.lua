include "dependencies.lua"

workspace "GameGuy"
	architecture "x86_64"
	startproject "GBZ80"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
	include "GameGuy/vendor/Glad"
	include "GameGuy/vendor/GLFW"
	include "GameGuy/vendor/imgui"
	include "GameGuy/vendor/stb"
group ""

include "GBZ80"
include "GameGuy"