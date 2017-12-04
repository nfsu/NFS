#pragma once

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "NFS.lib")

#ifdef _DEBUG
#pragma comment(lib, "Qt5Cored.lib")
#pragma comment(lib, "Qt5Widgetsd.lib")
#pragma comment(lib, "Qt5Guid.lib")
#pragma comment(lib, "Qt5OpenGLd.lib")
#pragma comment(lib, "libEGLd.lib")
#else
#pragma comment(lib, "Qt5Core.lib")
#pragma comment(lib, "Qt5Widgets.lib")
#pragma comment(lib, "Qt5Gui.lib")
#pragma comment(lib, "Qt5OpenGL.lib")
#pragma comment(lib, "libEGL.lib")
#endif

#ifdef _WIN32
//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif