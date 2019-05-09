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
#ifndef __BASLERREAMER_CAMERA_H__
#define __BASLERREAMER_CAMERA_H__

#include <string>

#include <pylon/PylonIncludes.h>
#include "cudaUtility.h"

using namespace Pylon;
using namespace std;


/*******
 * Basler GIGE Camera Driver program
 * @AmaZzz
 */ 
class baslerCamera
{
private:
	static int exitCode;
	//constructor
	baslerCamera();

public:
	//static CGrabResultPtr camera_data();
	//static baslerCamera* baslerCamera::Create();
	static baslerCamera* Create();
	CInstantCamera Create_Pylon();
	CGrabResultPtr camera_data();
	//convert the basler_camera output formate MONO to RGBA
	bool convert_MONO_ImageRGBA(float4** Save_image, int memory_distribute_code, CGrabResultPtr ptrGrabResult, uint8_t *pBuffer, float4** cpu, float4** gpu, int* width, int* height);
	//Destroy
	~baslerCamera();
    
};


#endif

