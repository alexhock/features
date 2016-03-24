################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../CFitsImages.cpp \
../main.cpp \
../zz_main.cpp 

OBJS += \
./CFitsImages.o \
./main.o \
./zz_main.o 

CPP_DEPS += \
./CFitsImages.d \
./main.d \
./zz_main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/ah14aeb/Software/cfitsio/include -I/home/ah14aeb/Software/opencv310/include/cudev -I/home/ah14aeb/Software/opencv310/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


