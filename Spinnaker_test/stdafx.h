#pragma once

#ifndef _DEBUG
#pragma comment(lib, "SpinnakerGPU_v141.lib")
#else
#pragma comment(lib, "SpinnakerGPUd_v141.lib")
#endif


#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"
#include <iostream>
#include <sstream>

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace std;