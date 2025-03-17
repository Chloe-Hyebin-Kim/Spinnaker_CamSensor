#include "Acquisition.h"


int Acquisition::RunSingleCamera(CameraPtr pCam)
{
	int result;

	try
	{
		// Retrieve TL device nodemap and print device information
		INodeMap& nodeMapTLDevice = pCam->GetTLDeviceNodeMap();

		result = PrintDeviceInfo(nodeMapTLDevice);

		// Initialize camera
		pCam->Init();

		// Retrieve GenICam nodemap
		INodeMap& nodeMap = pCam->GetNodeMap();

		// Configure heartbeat for GEV camera
#ifdef _DEBUG
		result = result | DisableGVCPHeartbeat(pCam);
#else
		result = result | ResetGVCPHeartbeat(pCam);
#endif
		// Set stream mode
		result = result | SetStreamMode(pCam);

		// Acquire images
		result = result | AcquireImages(pCam, nodeMap, nodeMapTLDevice);

#ifdef _DEBUG
		// Reset heartbeat for GEV camera
		result = result | ResetGVCPHeartbeat(pCam);
#endif

		// Deinitialize camera
		pCam->DeInit();
	}
	catch (Spinnaker::Exception& e)
	{
		cout << "Error: " << e.what() << endl;
		result = -1;
	}

	return result;
}


int Acquisition::SetStreamMode(CameraPtr pCam)
{
	int result = 0;

	// Retrieve Stream nodemap
	const INodeMap& sNodeMap = pCam->GetTLStreamNodeMap();

	// The node "StreamMode" is only available for GEV cameras.
	// Skip setting stream mode if the node is inaccessible.
	const CEnumerationPtr ptrStreamMode = sNodeMap.GetNode("StreamMode");
	if (!IsReadable(ptrStreamMode) || !IsWritable(ptrStreamMode))
	{
		return 0;
	}

	gcstring streamMode;
	switch (chosenStreamMode)
	{
	case STREAM_MODE_PGRLWF:
		streamMode = "LWF";
		break;
	case STREAM_MODE_SOCKET:
		streamMode = "Socket";
		break;
	case STREAM_MODE_TELEDYNE_GIGE_VISION:
	default:
		streamMode = "TeledyneGigeVision";
	}

	// Retrieve the desired entry node from the enumeration node
	const CEnumEntryPtr ptrStreamModeCustom = ptrStreamMode->GetEntryByName(streamMode);
	if (!IsReadable(ptrStreamModeCustom))
	{
		// Failed to get custom node
		cout << "Stream mode " + streamMode + " not available.  Aborting..." << endl;
		return -1;
	}
	// Retrieve the integer value from the entry node
	const int64_t streamModeCustom = ptrStreamModeCustom->GetValue();

	// Set integer as new value for enumeration node
	ptrStreamMode->SetIntValue(streamModeCustom);

	// Print out the current stream mode
	cout << endl << "Stream Mode set to " + ptrStreamMode->GetCurrentEntry()->GetSymbolic() << "..." << endl;

	return 0;
}

int Acquisition::AcquireImages(CameraPtr pCam, INodeMap& nodeMap, INodeMap& nodeMapTLDevice)
{
	int result = 0;

	cout << endl << endl << "*** IMAGE ACQUISITION ***" << endl << endl;

	try
	{
		//
		// Set acquisition mode to continuous
		//
		// *** NOTES ***
		// Because the example acquires and saves 10 images, setting acquisition
		// mode to continuous lets the example finish. If set to single frame
		// or multiframe (at a lower number of images), the example would just
		// hang. This would happen because the example has been written to
		// acquire 10 images while the camera would have been programmed to
		// retrieve less than that.
		//
		// Setting the value of an enumeration node is slightly more complicated
		// than other node types. Two nodes must be retrieved: first, the
		// enumeration node is retrieved from the nodemap; and second, the entry
		// node is retrieved from the enumeration node. The integer value of the
		// entry node is then set as the new value of the enumeration node.
		//
		// Notice that both the enumeration and the entry nodes are checked for
		// availability and readability/writability. Enumeration nodes are
		// generally readable and writable whereas their entry nodes are only
		// ever readable.
		//
		// Retrieve enumeration node from nodemap
		CEnumerationPtr ptrAcquisitionMode = nodeMap.GetNode("AcquisitionMode");
		if (!IsReadable(ptrAcquisitionMode) || !IsWritable(ptrAcquisitionMode))
		{
			cout << "Unable to set acquisition mode to continuous (enum retrieval). Aborting..." << endl << endl;
			return -1;
		}

		// Retrieve entry node from enumeration node
		CEnumEntryPtr ptrAcquisitionModeContinuous = ptrAcquisitionMode->GetEntryByName("Continuous");
		if (!IsReadable(ptrAcquisitionModeContinuous))
		{
			cout << "Unable to get or set acquisition mode to continuous (entry retrieval). Aborting..." << endl
				<< endl;
			return -1;
		}

		// Retrieve integer value from entry node
		const int64_t acquisitionModeContinuous = ptrAcquisitionModeContinuous->GetValue();

		// Set integer value from entry node as new value of enumeration node
		ptrAcquisitionMode->SetIntValue(acquisitionModeContinuous);

		cout << "Acquisition mode set to continuous..." << endl;

		//
		// Begin acquiring images
		//
		// *** NOTES ***
		// What happens when the camera begins acquiring images depends on the
		// acquisition mode. Single frame captures only a single image, multi
		// frame captures a set number of images, and continuous captures a
		// continuous stream of images. Because the example calls for the
		// retrieval of 10 images, continuous mode has been set.
		//
		// *** LATER ***
		// Image acquisition must be ended when no more images are needed.
		//
		pCam->BeginAcquisition();

		cout << "Acquiring images..." << endl;

		//
		// Retrieve device serial number for filename
		//
		// *** NOTES ***
		// The device serial number is retrieved in order to keep cameras from
		// overwriting one another. Grabbing image IDs could also accomplish
		// this.
		//
		gcstring deviceSerialNumber("");
		CStringPtr ptrStringSerial = nodeMapTLDevice.GetNode("DeviceSerialNumber");
		if (IsReadable(ptrStringSerial))
		{
			deviceSerialNumber = ptrStringSerial->GetValue();

			cout << "Device serial number retrieved as " << deviceSerialNumber << "..." << endl;
		}
		cout << endl;

		// Retrieve, convert, and save images
		const unsigned int k_numImages = 10;

		//
		// Create ImageProcessor instance for post processing images
		//
		ImageProcessor processor;

		//
		// Set default image processor color processing method
		//
		// *** NOTES ***
		// By default, if no specific color processing algorithm is set, the image
		// processor will default to NEAREST_NEIGHBOR method.
		//
		processor.SetColorProcessing(SPINNAKER_COLOR_PROCESSING_ALGORITHM_HQ_LINEAR);

		for (unsigned int imageCnt = 0; imageCnt < k_numImages; imageCnt++)
		{
			try
			{
				//
				// Retrieve next received image
				//
				// *** NOTES ***
				// Per default, the camera immediately transmits captured images
				// to the PC where they fill up Spinnaker's image buffers in the
				// PC's working memory.
				// Depending on the buffer handling mode (see example
				// BufferHandling), calling GetNextImage will return either the
				// newest or oldest image from these buffers.
				// If the buffers are empty when GetNextImage is called, the call
				// will hang until the next image arrives or until the timeout
				// value is reached.
				//
				// *** LATER ***
				// Once an image from the buffer is saved and/or no longer
				// needed, the image must be released in order to keep the
				// buffer from filling up.
				//
				ImagePtr pResultImage = pCam->GetNextImage(1000);

				//
				// Ensure image completion
				//
				// *** NOTES ***
				// Images can easily be checked for completion. This should be
				// done whenever a complete image is expected or required.
				// Further, check image status for a little more insight into
				// why an image is incomplete.
				//
				if (pResultImage->IsIncomplete())
				{
					// Retrieve and print the image status description
					cout << "Image incomplete: " << Image::GetImageStatusDescription(pResultImage->GetImageStatus())
						<< "..." << endl
						<< endl;
				}
				else
				{
					//
					// Print image information; height and width recorded in pixels
					//
					// *** NOTES ***
					// Images have quite a bit of available metadata including
					// things such as CRC, image status, and offset values, to
					// name a few.
					//
					const size_t width = pResultImage->GetWidth();

					const size_t height = pResultImage->GetHeight();

					cout << "Grabbed image " << imageCnt << ", width = " << width << ", height = " << height << endl;

					//
					// Convert image to mono 8
					//
					// *** NOTES ***
					// Images can be converted between pixel formats by using
					// the appropriate enumeration value. Unlike the original
					// image, the converted one does not need to be released as
					// it does not affect the camera buffer.
					//
					// When converting images, color processing algorithm is an
					// optional parameter.
					//
					ImagePtr convertedImage = processor.Convert(pResultImage, PixelFormat_Mono8);

					// Create a unique filename
					ostringstream filename;

					filename << "Acquisition-";
					if (!deviceSerialNumber.empty())
					{
						filename << deviceSerialNumber.c_str() << "-";
					}
					filename << imageCnt << ".jpg";

					//
					// Save image
					//
					// *** NOTES ***
					// The standard practice of the examples is to use device
					// serial numbers to keep images of one device from
					// overwriting those of another.
					//
					convertedImage->Save(filename.str().c_str());

					cout << "Image saved at " << filename.str() << endl;
				}

				//
				// Release image
				//
				// *** NOTES ***
				// Images retrieved directly from the camera (i.e. non-converted
				// images) need to be released in order to keep them from filling
				// the buffers.
				//
				pResultImage->Release();

				cout << endl;
			}
			catch (Spinnaker::Exception& e)
			{
				cout << "Error: " << e.what() << endl;
				result = -1;
			}
		}

		//
		// End acquisition
		//
		// *** NOTES ***
		// Ending acquisition appropriately helps ensure that devices clean up
		// properly and do not need to be power-cycled to maintain integrity.
		//

		pCam->EndAcquisition();
	}
	catch (Spinnaker::Exception& e)
	{
		cout << "Error: " << e.what() << endl;
		return -1;
	}

	return result;
}


int Acquisition::ConfigureGVCPHeartbeat(CameraPtr pCam, bool enableHeartbeat)
{
	//
	// Write to boolean node controlling the camera's heartbeat
	//
	// *** NOTES ***
	// This applies only to GEV cameras.
	//
	// GEV cameras have a heartbeat built in, but when debugging applications the
	// camera may time out due to its heartbeat. Disabling the heartbeat prevents
	// this timeout from occurring, enabling us to continue with any necessary
	// debugging.
	//
	// *** LATER ***
	// Make sure that the heartbeat is reset upon completion of the debugging.
	// If the application is terminated unexpectedly, the camera may not locked
	// to Spinnaker indefinitely due to the the timeout being disabled.  When that
	// happens, a camera power cycle will reset the heartbeat to its default setting.

	// Retrieve TL device nodemap
	INodeMap& nodeMapTLDevice = pCam->GetTLDeviceNodeMap();

	// Retrieve GenICam nodemap
	INodeMap& nodeMap = pCam->GetNodeMap();

	CEnumerationPtr ptrDeviceType = nodeMapTLDevice.GetNode("DeviceType");
	if (!IsReadable(ptrDeviceType))
	{
		return -1;
	}

	if (ptrDeviceType->GetIntValue() != DeviceType_GigEVision)
	{
		return 0;
	}

	if (enableHeartbeat)
	{
		cout << endl << "Resetting heartbeat..." << endl << endl;
	}
	else
	{
		cout << endl << "Disabling heartbeat..." << endl << endl;
	}

	CBooleanPtr ptrDeviceHeartbeat = nodeMap.GetNode("GevGVCPHeartbeatDisable");
	if (!IsWritable(ptrDeviceHeartbeat))
	{
		cout << "Unable to configure heartbeat. Continuing with execution as this may be non-fatal..." << endl << endl;
	}
	else
	{
		ptrDeviceHeartbeat->SetValue(!enableHeartbeat);

		if (!enableHeartbeat)
		{
			cout << "WARNING: Heartbeat has been disabled for the rest of this example run." << endl;
			cout << "         Heartbeat will be reset upon the completion of this run.  If the " << endl;
			cout << "         example is aborted unexpectedly before the heartbeat is reset, the" << endl;
			cout << "         camera may need to be power cycled to reset the heartbeat." << endl << endl;
		}
		else
		{
			cout << "Heartbeat has been reset." << endl;
		}
	}

	return 0;
}

int Acquisition::ResetGVCPHeartbeat(CameraPtr pCam)
{
	return ConfigureGVCPHeartbeat(pCam, true);
}

int Acquisition::DisableGVCPHeartbeat(CameraPtr pCam)
{
	return ConfigureGVCPHeartbeat(pCam, false);
}


int Acquisition::PrintDeviceInfo(INodeMap& nodeMap)
{
	int result = 0;
	cout << endl << "*** DEVICE INFORMATION ***" << endl << endl;

	try
	{
		FeatureList_t features;
		const CCategoryPtr category = nodeMap.GetNode("DeviceInformation");
		if (IsReadable(category))
		{
			category->GetFeatures(features);

			for (auto it = features.begin(); it != features.end(); ++it)
			{
				const CNodePtr pfeatureNode = *it;
				cout << pfeatureNode->GetName() << " : ";
				CValuePtr pValue = static_cast<CValuePtr>(pfeatureNode);
				cout << (IsReadable(pValue) ? pValue->ToString() : "Node not readable");
				cout << endl;
			}
		}
		else
		{
			cout << "Device control information not available." << endl;
		}
	}
	catch (Spinnaker::Exception& e)
	{
		cout << "Error: " << e.what() << endl;
		result = -1;
	}

	return result;
}