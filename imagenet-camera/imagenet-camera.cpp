/*
 * Copyright (c) 2017, NVIDIA CORPORATION. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

//#include "gstCamera.h"

#include "glDisplay.h"
#include "glTexture.h"

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "cudaNormalize.h"
#include "cudaFont.h"
#include "imageNet.h"

/**************Basler Camera driver program********
 **************************************************/
/*
extern "C++"
{
#include "baslerCamera.h"
}*/

#include <pylon/PylonIncludes.h>

using namespace Pylon;
using namespace std;

using namespace GenApi;

#include "cudaMappedMemory.h"

#include "cudaUtility.h"

#define DEFAULT_CAMERA -1	// -1 for onboard camera, or change to index of /dev/video V4L2 camera (>=0)	
		
		

// loadImageRGBA   *gpu=*imgCUDA is the output    ** Save_image mean define a pointer which point to the pointer
bool convert_MONO_ImageRGBA(float4** Save_image, int memory_distribute_code, CGrabResultPtr ptrGrabResult, uint8_t *pBuffer, float4** cpu, float4** gpu, int* width, int* height )
{
	const uint32_t imgWidth  = ptrGrabResult->GetWidth();
	const uint32_t imgHeight = ptrGrabResult->GetHeight();
	const uint32_t imgPixels = imgWidth * imgHeight;
	const size_t   imgSize   = imgWidth * imgHeight * sizeof(float) * 4;
	//printf("[Address]The address of the global ppppppppppointer1 is ===>%p\n", *Save_image);
	printf("\n[loaded image]  %s  (%u x %u)  %zu bytes\n", "The_Grabbed_image_is:::", imgWidth, imgHeight, imgSize);
	
	// allocate buffer for the image only one pBuffer otherwise the memory will run out...(will distribute different address)
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
										  float(1.0));//the 4th represent the RGBA A(Alpha)--transparency
			
			cpuPtr[y*imgWidth+x] = px;//The px is the pixel of the picture,contains px.x/y/z/w
			//printf("==========================Test_Flag_For_x_Loop%d======y=>%d =====>%f \n", x, y, float(*(pBuffer+y*imgWidth+x)) );
		}
	}
	
	*width  = imgWidth;
	*height = imgHeight;
	return true;	
}



int main( int argc, char** argv )//本例中参数只有运行命令并无其它--参数，argc==1,argv==./imagenet-camera
{
	printf("imagenet-camera\n  args (%i):  ", argc);
	for( int i=0; i < argc; i++ )
		printf("%i [%s]  ", i, argv[i]);
		
	printf("\n\n");
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
	 * create the basler camera device ==============>相机数据初始化（从相机拿数据）
	 */
	int exitCode = 0;
        PylonInitialize();
        
	//Create an instant camera object with the camera device found first.
	CInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());	
		
	//Print the model name of the camerea
	std::cout << "Using device "<< camera.GetDeviceInfo().GetModelName() << std::endl;
	//you can fill one const number in(),then it will grab number flame.Otherwise, 
	//if the() is empty, it will grabbing continously.
	camera.MaxNumBuffer = 25;//The output buffer number ,using to store the Image buffer
	
	INodeMap& nodemap = camera.GetNodeMap();
	
	camera.Open();
	/*
	// Set the auto function profile to Gain Minimum
	CEnumerationPtr(nodemap.GetNode("AutoFunctionProfile"))->FromString("GainMinimum");
	// Set the auto function profile to Exposure Minimum
	CEnumerationPtr(nodemap.GetNode("AutoFunctionProfile"))->FromString("ExposureMinimum");
	// Enable Gain and Exposure Auto auto functions and set the operating mode to Continuous
	CEnumerationPtr(nodemap.GetNode("GainAuto"))->FromString("Continuous");
	CEnumerationPtr(nodemap.GetNode("ExposureAuto"))->FromString("Continuous");
	*/
	//set the required center position then the OffsetX/OffsetY will be adjusted automatically
	// Enable Center X
	CBooleanPtr(nodemap.GetNode("CenterX"))->SetValue(true);
	// Enable Center Y
	CBooleanPtr(nodemap.GetNode("CenterY"))->SetValue(true);

	CIntegerPtr width( nodemap.GetNode( "Width"));
        CIntegerPtr height( nodemap.GetNode( "Height"));
	
	int64_t set_Width = 640;//280*300
        //newWidth = Adjust(newWidth, width->GetMin(), width->GetMax(), width->GetInc());

        int64_t set_Height = 480;
        //newHeight = Adjust(newHeight, height->GetMin(), height->GetMax(), height->GetInc());

        width->SetValue(set_Width);
        height->SetValue(set_Height);

	camera.StartGrabbing();//this command will open camera automatically
	
	//this smart pointer is used t receive the grab result data
	CGrabResultPtr ptrGrabResult;
/*
	//baslerCamera *cam = new baslerCamera();// cannnot directly use the object visit the private function/variable!!!
	baslerCamera* cam = baslerCamera::Create();
	
	CInstantCamera camera = cam->Create_Pylon();
	
	CGrabResultPtr ptrGrabResult = cam->camera_data();
	*/
	/*
	 * create imageNet  ==============>创建网络，并用nvcaffe parser解析caffe网络和数据
	 */
	printf("basler_initial_over\n");											
	//original function: static imageNet* Create( NetworkType networkType=GOOGLENET, uint32_t maxBatchSize=2 );
	imageNet* net = imageNet::Create(argc, argv);
	
	if( !net )
	{
		printf("imagenet-console:   failed to initialize imageNet\n");
		return 0;
	}

	/*
	 * create openGL window  ==============>显示界面与数据，！！这里可以拿到分类的数据并进行后期处理
	 * 										但是这里显示界面显示的却是Alexnet???????????
	 */
	glDisplay* display = glDisplay::Create();//******将返回一个vp对象（glDisplay* vp）
	glTexture* texture = NULL;
	
	if( !display ) {
		printf("\nimagenet-camera:  failed to create openGL display\n");
	}
	else
	{/////////////////////////1280, 1024,////ptrGrabResult->GetWidth()/ptrGrabResult->GetHeight()/////??????????????????????need a const///////////////////////////////////////////
		texture = glTexture::Create(set_Width, set_Height, GL_RGBA32F_ARB/*GL_RGBA8*/);//Return True

		if( !texture )
			printf("imagenet-camera:  failed to create openGL texture\n");
		else
		    printf("imagenet-camera:Created openGL texture\n");
	}
	
	
	/*
	 * create font   创建文字
	 */
	cudaFont* font = cudaFont::Create();
	//printf("[Test_Flag]::::::::::confirm cudaFont crate font successful\n");
	/*
	 * processing loop 执行视频流处理
	 */
	float confidence = 0.0f;
	int memory_distribute_flag = true; 
	float4* Save_image = NULL;
// 	double C_frame = CFloatPtr(nodemap.GetNode("ResultingFrameRate"))->GetValue();
// 	printf("\n\n\nFrame=================> %d\n\n\n", C_frame);
	
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!	
	while( camera.IsGrabbing())//循环读取每一帧数据
 	{
// 	        printf("[Test_Flag]::::::::::confirm get into while loop\n");
 		float* imgCPU  = NULL;
 		float* imgCUDA = NULL;
 		uint8_t *pImageBuffer = NULL;
// 		// convert from YUV to RGBA
 		void* imgRGBA = NULL;
		int    imgWidth  = 0;
		int    imgHeight = 0;
 		int jj=0;
// 		
		//get the resulting frame rate
		
// 		if( !camera->ConvertRGBA(imgCUDA, &imgRGBA) )
// 			printf("imagenet-camera:  failed to convert from NV12 to RGBA\n");
// 	    /////////////////////////////////////////////////////////////////////////
		
		camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);
		
		if(ptrGrabResult->GrabSucceeded())
		{
			//make sure the picture is grabbed successfully
		        //CImagePersistence::Save( ImageFileFormat_Png, "GrabbedImage.png", ptrGrabResult);
			//CImagePersistence::Save( ImageFileFormat_Png, "GrabbedImage.png", ptrGrabResult);
			uint8_t *pImageBuffer = (uint8_t *) ptrGrabResult->GetBuffer();;
// 			while(*(pImageBuffer+jj) != '\0')
// 			{
// 			    printf("DATA[%d]= %d\n", jj, *(pImageBuffer+jj));
// 			    jj++;
// 			}
			if(convert_MONO_ImageRGBA((float4**)&Save_image, memory_distribute_flag, ptrGrabResult, pImageBuffer,(float4**)&imgCPU, (float4**)&imgCUDA, &imgWidth, &imgHeight) != 0)
			{
			    printf("convert image from MONO to RGBA Failed\n");
			}
// 			//get the image buffer
// 			while(pImageBuffer[jj] != '\0')
// 			{
// 			    printf("DATA[%d]= %d", jj, pImageBuffer[jj]);
//			    jj++;
//			}
//			printf("the_number_of_pImageBuffer:%d", jj);
			// classify image！！！！！！！！！！！读取输出的class信息并答应出来
			//Function：return classsIndex(最大可能性的那一类数值索引（eg:0631）)；给confidence赋值为最大一类的概率是多少（softmax层的最大值）
// 			printf("[Address]The address of the global pointer is [main] ===>%p\n", Save_image);
// 			printf("[imgCUDA-Address]The address of the global pointer is [main] ===>%p\n", imgCUDA);
			const int img_class = net->Classify((float*)Save_image, ptrGrabResult->GetWidth(), ptrGrabResult->GetHeight(), &confidence);
			
			printf("\n{The_Result_of_the_confidence}::::::::::%6f\n", confidence * 100.0f);
			if( img_class >= 0 )
			{
				printf("[Classify_Reuslt]  imagenet-camera:  %2.5f%% class #%i (%s)\n", confidence * 100.0f, img_class, net->GetClassDesc(img_class));	

				if( font != NULL )//在图片上打印出类别信息和分类准确率
				{
					char str[256];
					//restore the str with confidence and GetClassDesc
					//Format of the function
					//int sprintf(char *buffer, const char *format, [argument]...)
					/*
					sprintf(str, "%s","NanChang University Compute Vision Test Demo");
					font->RenderOverlay((float4*)Save_image, (float4*)Save_image, ptrGrabResult->GetWidth(), ptrGrabResult->GetHeight(),
									    str, 0, 0, make_float4(0.0f, 0.0f, 255.0f, 255.0f));//The color of red is 255,0,0
					*/
					sprintf(str, "%s%s %s%-05.2f%%","NCU CV Demo Classiy_Result:", net->GetClassDesc(img_class), "Value=", confidence * 100.0f);
					font->RenderOverlay((float4*)Save_image, (float4*)Save_image, ptrGrabResult->GetWidth(), ptrGrabResult->GetHeight(),
									    str, 0, 0, make_float4(255.0f, 0.0f, 0.0f, 255.0f));//The color of red is 255,0,0
				}
				
				if( display != NULL )//在窗口的toolbar上打印出信息
				{
					char str[256];
					//FPS = 1000000000.0f / mAvgTime
					sprintf(str, "TensorRT build %i.%i.%i | %s | %s | %04.1f FPS", NV_TENSORRT_MAJOR, NV_TENSORRT_MINOR, NV_TENSORRT_PATCH, net->GetNetworkName(), net->HasFP16() ? "FP16" : "FP32", display->GetFPS());
					//sprintf(str, "TensorRT build %x | %s | %04.1f FPS | %05.2f%% %s", NV_GIE_VERSION, net->GetNetworkName(), display->GetFPS(), confidence * 100.0f, net->GetClassDesc(img_class));
					display->SetTitle(str);	
				}	
			}

			// update display 更新显示信息
			if( display != NULL )
			{
				display->UserEvents();
				display->BeginRender();

				if( texture != NULL )
				{
					// rescale image pixel intensities for display
					CUDA(cudaNormalizeRGBA((float4*)Save_image, make_float2(0.0f, 255.0f), 
									(float4*)Save_image, make_float2(0.0f, 1.0f), 
									ptrGrabResult->GetWidth(), ptrGrabResult->GetHeight()));

					// map from CUDA to openGL using GL interop
					void* tex_map = texture->MapCUDA();

					if( tex_map != NULL )
					{
						cudaMemcpy(tex_map, Save_image, texture->GetSize(), cudaMemcpyDeviceToDevice);
						texture->Unmap();
					}

					// draw the texture
					texture->Render(100,100);		
				}

				display->EndRender();
			}
			
		}
		
		else 
		{
		    std::cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << std::endl;
		    
		}
		
 		delete pImageBuffer;
 		pImageBuffer = NULL;
		
		memory_distribute_flag = false;//flag used to distribute memory to Save_image only once.
	}
	
	printf("\nimagenet-camera:  un-initializing video device\n");
	
///////////////////////////////////////////////////////////////////	
	/*
	 * shutdown the camera device
	 */
// 	if( cam != NULL )
// 	{
// 		delete cam;
// 		cam = NULL;
// 	}

	if( display != NULL )
	{
		delete display;
		display = NULL;
	}
	
	printf("imagenet-camera:  video device has been un-initialized.\n");
	printf("imagenet-camera:  this concludes the test of the video device.\n");
	return 0;
}

