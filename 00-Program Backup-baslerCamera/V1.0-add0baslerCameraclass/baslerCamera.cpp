// /*
//  * Copyright (c) 2017, NVIDIA CORPORATION. All rights reserved.
//  *
//  * Permission is hereby granted, free of charge, to any person obtaining a
//  * copy of this software and associated documentation files (the "Software"),
//  * to deal in the Software without restriction, including without limitation
//  * the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  * and/or sell copies of the Software, and to permit persons to whom the
//  * Software is furnished to do so, subject to the following conditions:
//  *
//  * The above copyright notice and this permission notice shall be included in
//  * all copies or substantial portions of the Software.
//  *
//  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
//  * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  * DEALINGS IN THE SOFTWARE.
//  */
// 
#include "baslerCamera.h"

#include <string.h>

#include "cudaMappedMemory.h"

//#include <pylon/PylonIncludes.h>
//using namespace Pylon;
//using namespace std;


baslerCamera::baslerCamera()
{
    exitCode = 0;
//    int8_t imageoutput_count = 0;
    
}

//destructor
baslerCamera::~baslerCamera()
{

}

baslerCamera* baslerCamera::Create()
{
    baslerCamera *cam = new baslerCamera();
    return cam;
}



CInstantCamera baslerCamera::Create_Pylon()
{
    PylonInitialize();
    try
    {

	//Create an instant camera object with the camera device found first.
	CInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());
	
	//Print the model name of the camerea
	cout << "Using device "<< camera.GetDeviceInfo().GetModelName() << endl;
	
	//you can fill one const number in(),then it will grab number flame.Otherwise, 
	//if the() is empty, it will grabbing continously.
	camera.StartGrabbing();
	return camera;
    }
    
    catch (const GenericException &e)
    {
	cerr <<"An exception occured."<<endl
	<< e.GetDescription()<<endl;
	exitCode = 1;
    }
    
}

CGrabResultPtr baslerCamera::camera_data()
{
	//this smart pointer is used t receive the grab result data
	CGrabResultPtr ptrGrabResult;
	return ptrGrabResult;
}
/*
CGrabResultPtr baslerCamera::basler_create()
{
    int exitCode = 0;
    PylonInitialize();
    try
    {

	//Create an instant camera object with the camera device found first.
	CInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());
	
	//Print the model name of the camerea
	cout << "Using device "<< camera.GetDeviceInfo().GetModelName() << endl;
	
	//you can fill one const number in(),then it will grab number flame.Otherwise, 
	//if the() is empty, it will grabbing continously.
	camera.StartGrabbing();
	//this smart pointer is used t receive the grab result data
	CGrabResultPtr ptrGrabResult;
	return ptrGrabResult;
    }
    
    catch (const GenericException &e)
    {
	cerr <<"An exception occured."<<endl
	<< e.GetDescription()<<endl;
	exitCode = 1;
    }   
}*/

// loadImageRGBA   *gpu=*imgCUDA is the output
bool baslerCamera::convert_MONO_ImageRGBA(float4** Save_image, int memory_distribute_code, CGrabResultPtr ptrGrabResult, uint8_t *pBuffer, float4** cpu, float4** gpu, int* width, int* height )
{
	const uint32_t imgWidth  = ptrGrabResult->GetWidth();
	const uint32_t imgHeight = ptrGrabResult->GetHeight();
	const uint32_t imgPixels = imgWidth * imgHeight;
	const size_t   imgSize   = imgWidth * imgHeight * sizeof(float) * 4;
	printf("[Address]The address of the global ppppppppppointer1 is ===>%p\n", *Save_image);
	printf("[loaded image]  %s  (%u x %u)  %zu bytes\n", "The_Grabbed_image_is:::", imgWidth, imgHeight, imgSize);
	
	// allocate buffer for the image only one pBuffer
	if(memory_distribute_code != 0)
	{
	    printf("[Test_Flag]======================>check memory distribute once\n");
	    if( !cudaAllocMapped((void**)cpu, (void**)gpu, imgSize) )
	    {
		printf(LOG_CUDA "failed to allocated %zu bytes for image \n", imgSize);
		return false;
	    }
	    *Save_image = *gpu;

	}
	
	printf("[Address]The address of the global ppppppppppointer2 is ===>%p\n", *Save_image);
	float4* cpuPtr = *Save_image;

	for( uint32_t y=0; y < imgHeight; y++ )
	{
		for( uint32_t x=0; x < imgWidth; x++ )
		{
			//
			//const QRgb rgb  = qImg.pixel(x,y);   float(*(pBuffer+y*imgWidth+x))
			const float4 px = make_float4(float(*(pBuffer+y*imgWidth+x)), 
										  float(*(pBuffer+y*imgWidth+x)), 
										  float(*(pBuffer+y*imgWidth+x)),
										  float(0));
			
			cpuPtr[y*imgWidth+x] = px;//The px is the pixel of the picture,contains px.x/y/z/w
			//printf("==========================Test_Flag_For_x_Loop%d======y=>%d =====>%f \n", x, y, float(*(pBuffer+y*imgWidth+x)) );
		}
	}
	
	*width  = imgWidth;
	*height = imgHeight;
	printf("[Address]The address of the global ppppppppppointer3 is ===>%p\n", *Save_image);
	return true;
	
}

