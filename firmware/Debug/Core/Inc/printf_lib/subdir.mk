################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Inc/printf_lib/printf.c 

OBJS += \
./Core/Inc/printf_lib/printf.o 

C_DEPS += \
./Core/Inc/printf_lib/printf.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Inc/printf_lib/%.o Core/Inc/printf_lib/%.su Core/Inc/printf_lib/%.cyclo: ../Core/Inc/printf_lib/%.c Core/Inc/printf_lib/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G070xx -c -I../Core/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G0xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Inc-2f-printf_lib

clean-Core-2f-Inc-2f-printf_lib:
	-$(RM) ./Core/Inc/printf_lib/printf.cyclo ./Core/Inc/printf_lib/printf.d ./Core/Inc/printf_lib/printf.o ./Core/Inc/printf_lib/printf.su

.PHONY: clean-Core-2f-Inc-2f-printf_lib

