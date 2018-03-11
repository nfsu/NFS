#pragma once

#define USE_TIMER
#define USE_CATCH

#ifndef USE_CONSOLE
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif