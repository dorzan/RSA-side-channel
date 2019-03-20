################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ff.c \
../src/fr.c \
../src/l1.c \
../src/l1i.c \
../src/l3.c \
../src/pda.c \
../src/symbol.c \
../src/symbol_bfd.c \
../src/symbol_mach.c \
../src/timestats.c \
../src/util.c \
../src/vlist.c 

O_SRCS += \
../src/ff.o \
../src/fr.o \
../src/l1.o \
../src/l1i.o \
../src/l3.o \
../src/pda.o \
../src/symbol.o \
../src/timestats.o \
../src/util.o \
../src/vlist.o 

OBJS += \
./src/ff.o \
./src/fr.o \
./src/l1.o \
./src/l1i.o \
./src/l3.o \
./src/pda.o \
./src/symbol.o \
./src/symbol_bfd.o \
./src/symbol_mach.o \
./src/timestats.o \
./src/util.o \
./src/vlist.o 

C_DEPS += \
./src/ff.d \
./src/fr.d \
./src/l1.d \
./src/l1i.d \
./src/l3.d \
./src/pda.d \
./src/symbol.d \
./src/symbol_bfd.d \
./src/symbol_mach.d \
./src/timestats.d \
./src/util.d \
./src/vlist.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


