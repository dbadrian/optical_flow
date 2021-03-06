
find_package(Glog REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(Boost COMPONENTS system timer program_options REQUIRED)
find_package(FFTW3 COMPONENTS single REQUIRED)
find_package(ZLIB REQUIRED)
find_package(CUDA)

add_library(optical_flow
    Filter.cpp
    FilterFactory.cpp
    FourierTransformerFFTW.cpp
)

add_dependencies(optical_flow edvstools)
target_link_libraries(optical_flow
    PRIVATE 
        ${GLOG_LIBRARIES}
        ${Boost_LIBRARIES}
        ${EDVSTOOLS_LIB}
        ${FFTW3_LIBRARIES}
        ${ZLIB_LIBRARIES}
)

target_include_directories(optical_flow
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${EIGEN3_INCLUDE_DIR}
    PRIVATE
        ${GLOG_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
        ${FFTW3_INCLUDE_DIRS}
        ${ZLIB_INCLUDE_DIRS}
)



if(WITH_CUDA)
    add_library(optical_flow_gpu_utils DeviceBlob.cpp FourierTransformerCUFFTW.cpp)
    cuda_add_library(optical_flow_gpu GpuMath.cu)

    target_include_directories(optical_flow_gpu_utils PUBLIC ${EIGEN3_INCLUDE_DIR})

    target_link_libraries(optical_flow
        PUBLIC
            optical_flow_gpu
            optical_flow_gpu_utils
            ${CUDA_LIBRARIES}
            ${CUDA_CUBLAS_LIBRARIES}
            ${CUDA_CUFFT_LIBRARIES}
            ${CUDA_CUFFTW_LIBRARIES}
    )
#    target_link_libraries(optical_flow
#        PUBLIC optical_flow_gpu ${CUDA_LIBRARIES} ${CUDA_CUBLAS_LIBRARIES}
#    )
    target_include_directories(optical_flow PUBLIC ${CUDA_INCLUDE_DIRS})
endif()
