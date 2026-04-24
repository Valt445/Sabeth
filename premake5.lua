-- premake5.lua
workspace "Sabeth"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "Sabeth"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include "Walnut/WalnutExternal.lua"

include "Sabeth"