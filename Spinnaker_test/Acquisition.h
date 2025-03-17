#pragma once

#include "stdafx.h"

// Use the following enum to select the stream mode
enum StreamMode
{
	STREAM_MODE_TELEDYNE_GIGE_VISION, // Teledyne Gige Vision is the default stream mode for spinview which is supported on Windows
	STREAM_MODE_PGRLWF, // Light Weight Filter driver is our legacy driver which is supported on Windows
	STREAM_MODE_SOCKET, // Socket is supported for MacOS and Linux, and uses native OS network sockets instead of a
						// filter driver
};

#if defined(WIN32) || defined(WIN64)
const StreamMode chosenStreamMode = STREAM_MODE_TELEDYNE_GIGE_VISION;
#else
const StreamMode chosenStreamMode = STREAM_MODE_SOCKET;
#endif


class Acquisition
{
public:
	Acquisition() {};
	~Acquisition() {};

public:
	int RunSingleCamera(CameraPtr pCam);


private:
	// demonstrates how we can change stream modes.
	int SetStreamMode(CameraPtr pCam);

	// acquires and saves 10 images from a device.
	int AcquireImages(CameraPtr pCam, INodeMap& nodeMap, INodeMap& nodeMapTLDevice);

	// Disables or enables heartbeat on GEV cameras so debugging does not incur timeout errors
	int ConfigureGVCPHeartbeat(CameraPtr pCam, bool enableHeartbeat);
	int ResetGVCPHeartbeat(CameraPtr pCam);
	int DisableGVCPHeartbeat(CameraPtr pCam);

	// prints the device information of the camera from the transport layer;
	int PrintDeviceInfo(INodeMap& nodeMap);
};