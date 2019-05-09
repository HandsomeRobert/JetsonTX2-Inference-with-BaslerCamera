
file(GLOB imagenetCameraSources *.cpp)
file(GLOB imagenetCameraIncludes *.h )
# include_directories(/home/nvidia/jetson-inference/pylon_lib64)

cuda_add_executable(imagenet-camera ${imagenetCameraSources})
target_link_libraries(imagenet-camera nvcaffe_parser nvinfer jetson-inference)
