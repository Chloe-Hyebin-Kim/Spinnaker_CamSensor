#pragma once

#ifndef _DEBUG
//#pragma comment(lib, "SpinnakerGPU_v141.lib")
#pragma comment(lib, "Spinnakerd_v140.lib")
#else
//#pragma comment(lib, "SpinnakerGPUd_v141.lib")
#pragma comment(lib, "Spinnakerd_v140.lib")
#endif

#define _CRT_SECURE_NO_WARNINGS

#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE 101
#define _APS_NEXT_COMMAND_VALUE  40001
#define _APS_NEXT_CONTROL_VALUE  1001
#define _APS_NEXT_SYMED_VALUE    101
#endif
#endif

#include <iostream>
#include <sstream>
#include <tchar.h>

using namespace std;

#include <SDKDDKVer.h>

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
