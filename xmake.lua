add_rules("mode.debug", "mode.release")
set_languages("clatest", "c++latest")

set_toolchains("clang")
set_config("mode", "debug")
set_warnings("all", "error")

add_rules("plugin.compile_commands.autoupdate", { outputdir = "build" })
add_requires("SDL3", { system = true })

target("toys", function()
	set_kind("binary")
	add_includedirs("include")
	add_packages("SDL3")
	add_files("src/**.c")
end)
