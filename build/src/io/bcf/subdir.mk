################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/io/bcf/BCFEntry.cpp \
../src/io/bcf/BCFReader.cpp 

OBJS += \
./src/io/bcf/BCFEntry.o \
./src/io/bcf/BCFReader.o 

CPP_DEPS += \
./src/io/bcf/BCFEntry.d \
./src/io/bcf/BCFReader.d 


# Each subdirectory must supply rules for building sources it contributes
src/io/bcf/%.o: ../src/io/bcf/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -O3 -fno-strict-aliasing -march=native -mtune=native -ftree-vectorize -pipe -fomit-frame-pointer -flto -frename-registers -funroll-loops -fuse-linker-plugin -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

