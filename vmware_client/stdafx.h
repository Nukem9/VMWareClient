#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#include "dbgexts.h"

#include "VmWare.h"
#include "PhysAMD64.h"
#include "VmIPC.h"

// Detours
#pragma comment(lib, "detours/detours64.lib")
#include "detours/Detours.h"