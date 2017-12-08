################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/cr_startup_lpc17.c \
../src/main.c 

S_SRCS += \
../src/distrBF.s 

OBJS += \
./src/cr_startup_lpc17.o \
./src/distrBF.o \
./src/main.o 

C_DEPS += \
./src/cr_startup_lpc17.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__CODE_RED -D__REDLIB__ -D__USE_CMSIS=CMSISv1p30_LPC17xx -I"C:\Users\Siidheesh\OneDrive - National University of Singapore\NUS\Sem 3\EE2024\workspace\Lib_EaBaseBoard\inc" -I"C:\Users\Siidheesh\OneDrive - National University of Singapore\NUS\Sem 3\EE2024\workspace\Lib_CMSISv1p30_LPC17xx\inc" -I"C:\Users\Siidheesh\OneDrive - National University of Singapore\NUS\Sem 3\EE2024\workspace\Lib_MCU\inc" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -Wa,-ahlnds=$(basename $(notdir $@)).asm -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.s
	@echo 'Building file: $<'
	@echo 'Invoking: MCU Assembler'
	arm-none-eabi-gcc -c -x assembler-with-cpp -DDEBUG -D__CODE_RED -D__REDLIB__ -g3 -mcpu=cortex-m3 -mthumb -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


