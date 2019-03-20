################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../demo/FF-gnupg-1.4.13.c \
../demo/FR-1-file-access.c \
../demo/FR-2-file-access.c \
../demo/FR-flush.c \
../demo/FR-function-call-nodelay.c \
../demo/FR-function-call.c \
../demo/FR-gnupg-1.4.13.c \
../demo/FR-threshold.c \
../demo/FR-trace.c \
../demo/L1-capture.c \
../demo/L1-rattle.c \
../demo/L3-capture.c \
../demo/shm.c \
../demo/tool.c \
../demo/tool3.c 

O_SRCS += \
../demo/FF-gnupg-1.4.13.o \
../demo/FR-1-file-access.o \
../demo/FR-2-file-access.o \
../demo/FR-flush.o \
../demo/FR-function-call-nodelay.o \
../demo/FR-function-call.o \
../demo/FR-gnupg-1.4.13.o \
../demo/FR-threshold.o \
../demo/FR-trace.o \
../demo/L1-capture.o \
../demo/L1-rattle.o \
../demo/L3-capture.o \
../demo/shm.o \
../demo/tool.o \
../demo/tool3.o 

OBJS += \
./demo/FF-gnupg-1.4.13.o \
./demo/FR-1-file-access.o \
./demo/FR-2-file-access.o \
./demo/FR-flush.o \
./demo/FR-function-call-nodelay.o \
./demo/FR-function-call.o \
./demo/FR-gnupg-1.4.13.o \
./demo/FR-threshold.o \
./demo/FR-trace.o \
./demo/L1-capture.o \
./demo/L1-rattle.o \
./demo/L3-capture.o \
./demo/shm.o \
./demo/tool.o \
./demo/tool3.o 

C_DEPS += \
./demo/FF-gnupg-1.4.13.d \
./demo/FR-1-file-access.d \
./demo/FR-2-file-access.d \
./demo/FR-flush.d \
./demo/FR-function-call-nodelay.d \
./demo/FR-function-call.d \
./demo/FR-gnupg-1.4.13.d \
./demo/FR-threshold.d \
./demo/FR-trace.d \
./demo/L1-capture.d \
./demo/L1-rattle.d \
./demo/L3-capture.d \
./demo/shm.d \
./demo/tool.d \
./demo/tool3.d 


# Each subdirectory must supply rules for building sources it contributes
demo/%.o: ../demo/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


