if "%VM_BUILD%" == "" goto devBuild
gn gen out/Debug "--args=is_debug=true use_system_yasm=true is_official_build=false target_os=\"win\" target_cpu=\"x86\""
gn gen out/Release "--args=is_debug=false use_system_yasm=false is_official_build=true target_os=\"win\" target_cpu=\"x86\""
goto end
:devBuild
gn gen --ide=vs2013 out/Debug "--args=is_debug=true use_system_yasm=true is_official_build=false target_os=\"win\" target_cpu=\"x86\""
gn gen --ide=vs2013 out/Release "--args=is_debug=false use_system_yasm=false is_official_build=true target_os=\"win\" target_cpu=\"x86\""
:end