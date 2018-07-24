-- A solution
workspace "lua-opcua"
	configurations { "Debug", "Release"}

project "opcua"
	kind "SharedLib"
	language "C++"
	location "build_imx6"
	targetprefix ""
	targetdir "bin_imx6/%{cfg.buildcfg}"

	--includedirs { "/usr/include/lua5.3", ".", "../../" }
	includedirs { "/home/cch/mycode/skynet/3rd/lua/", "./src/", "../..", "../../build_imx6" }
	files { "./src/**.hpp", "./src/**.cpp"}

	-- buildoptions { '-Wno-unknown-warning', '-Wno-unknown-warning-option', '-Wall', '-Wextra', '-Wpedantic', '-pedantic', '-pedantic-errors', '-Wno-noexcept-type', '-std=c++14', '-ftemplate-depth=1024' }
	buildoptions { '-Wpedantic', '-pedantic', '-pedantic-errors', '-DSOL_NO_EXCEPTIONS=1', '-std=c++14', '-ftemplate-depth=2048', '-DUA_ARCHITECTURE_POSIX'}

	libdirs { "../../build_imx6/bin" }
	links { "pthread", "open62541" }
	linkoptions { "-Wl,--whole-archive -lmbedtls -lmbedx509 -lmbedcrypto -Wl,--no-whole-archive" }

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"

