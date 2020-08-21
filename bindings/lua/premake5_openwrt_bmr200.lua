-- A solution
workspace "lua-opcua"
	configurations { "Debug", "Release"}

project "opcua"
	kind "SharedLib"
	language "C++"
	location "build"
	targetprefix ""
	targetdir "bin/%{cfg.buildcfg}"

	--includedirs { "/usr/include/lua5.3", ".", "../../" }
	includedirs { "/home/cch/mycode/skynet/3rd/lua/", ".", "../..", "../../build_openwrt" }
	files { "./src/**.hpp", "./src/**.cpp"}

	-- buildoptions { '-Wno-unknown-warning', '-Wno-unknown-warning-option', '-Wall', '-Wextra', '-Wpedantic', '-pedantic', '-pedantic-errors', '-Wno-noexcept-type', '-std=c++14', '-ftemplate-depth=1024' }
	buildoptions { '-Wpedantic', '-pedantic', '-pedantic-errors', '-DSOL_NO_EXCEPTIONS=1', '-std=c++11', '-ftemplate-depth=4096', '-DUA_ARCHITECTURE_POSIX'}

	libdirs { "../../build_openwrt/bin" }
	links { "pthread", "open62541", "uuid", "ssl", "crypto" }
	linkoptions { "-fPIC" }

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"

