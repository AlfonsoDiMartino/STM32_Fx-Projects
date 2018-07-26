################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Utilities/Components/ts3510/ts3510.c 

OBJS += \
./Utilities/Components/ts3510/ts3510.o 

C_DEPS += \
./Utilities/Components/ts3510/ts3510.d 


# Each subdirectory must supply rules for building sources it contributes
Utilities/Components/ts3510/%.o: ../Utilities/Components/ts3510/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DSTM32F407VGTx -DSTM32F4 -DSTM32F4DISCOVERY -DSTM32 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -DUSE_RTOS_SYSTICK -I"/home/pietro/AC6workspace/FreeRTOS/inc" -I"/home/pietro/AC6workspace/FreeRTOS/CMSIS/core" -I"/home/pietro/AC6workspace/FreeRTOS/CMSIS/device" -I"/home/pietro/AC6workspace/FreeRTOS/Middlewares/Third_Party/FreeRTOS/Source/include" -I"/home/pietro/AC6workspace/FreeRTOS/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F" -I"/home/pietro/AC6workspace/FreeRTOS/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/STM32F4-Discovery" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/lsm303dlhc" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/exc7200" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/otm8009a" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/stmpe1600" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/n25q128a" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/lis3dsh" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/ts3510" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/cs43l22" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/Common" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/s25fl512s" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/st7789h2" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/s5k5cag" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/ili9341" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/ampire640480" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/mfxstm32l152" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/lis302dl" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/n25q256a" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/ili9325" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/st7735" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/stmpe811" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/wm8994" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/ampire480272" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/l3gd20" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/ov2640" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/ft6x06" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/n25q512a" -I"/home/pietro/AC6workspace/FreeRTOS/Utilities/Components/ls016b8uy" -I"/home/pietro/AC6workspace/FreeRTOS/HAL_Driver/Inc" -I"/home/pietro/AC6workspace/FreeRTOS/HAL_Driver/Inc/Legacy" -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


