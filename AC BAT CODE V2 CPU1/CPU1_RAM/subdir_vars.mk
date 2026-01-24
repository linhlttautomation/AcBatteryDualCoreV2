################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Add inputs and outputs from these tool invocations to the build variables 
CMD_SRCS += \
../2838x_RAM_SDFM_CLA_lnk_cpu1.cmd \
../f2838x_headers_nonBIOS_cpu1.cmd 

CLA_SRCS += \
../CPU1_Task.cla 

ASM_SRCS += \
../f2838x_codestartbranch.asm \
../f2838x_usdelay.asm 

C_SRCS += \
../CPU1_Main.c \
../f2838x_cputimers.c \
../f2838x_defaultisr.c \
../f2838x_epwm.c \
../f2838x_globalvariabledefs.c \
../f2838x_gpio.c \
../f2838x_piectrl.c \
../f2838x_pievect.c \
../f2838x_sdfm_drivers.c \
../f2838x_struct.c \
../f2838x_sysctrl.c 

CLA_DEPS += \
./CPU1_Task.d 

C_DEPS += \
./CPU1_Main.d \
./f2838x_cputimers.d \
./f2838x_defaultisr.d \
./f2838x_epwm.d \
./f2838x_globalvariabledefs.d \
./f2838x_gpio.d \
./f2838x_piectrl.d \
./f2838x_pievect.d \
./f2838x_sdfm_drivers.d \
./f2838x_struct.d \
./f2838x_sysctrl.d 

OBJS += \
./CPU1_Main.obj \
./CPU1_Task.obj \
./f2838x_codestartbranch.obj \
./f2838x_cputimers.obj \
./f2838x_defaultisr.obj \
./f2838x_epwm.obj \
./f2838x_globalvariabledefs.obj \
./f2838x_gpio.obj \
./f2838x_piectrl.obj \
./f2838x_pievect.obj \
./f2838x_sdfm_drivers.obj \
./f2838x_struct.obj \
./f2838x_sysctrl.obj \
./f2838x_usdelay.obj 

ASM_DEPS += \
./f2838x_codestartbranch.d \
./f2838x_usdelay.d 

OBJS__QUOTED += \
"CPU1_Main.obj" \
"CPU1_Task.obj" \
"f2838x_codestartbranch.obj" \
"f2838x_cputimers.obj" \
"f2838x_defaultisr.obj" \
"f2838x_epwm.obj" \
"f2838x_globalvariabledefs.obj" \
"f2838x_gpio.obj" \
"f2838x_piectrl.obj" \
"f2838x_pievect.obj" \
"f2838x_sdfm_drivers.obj" \
"f2838x_struct.obj" \
"f2838x_sysctrl.obj" \
"f2838x_usdelay.obj" 

C_DEPS__QUOTED += \
"CPU1_Main.d" \
"f2838x_cputimers.d" \
"f2838x_defaultisr.d" \
"f2838x_epwm.d" \
"f2838x_globalvariabledefs.d" \
"f2838x_gpio.d" \
"f2838x_piectrl.d" \
"f2838x_pievect.d" \
"f2838x_sdfm_drivers.d" \
"f2838x_struct.d" \
"f2838x_sysctrl.d" 

CLA_DEPS__QUOTED += \
"CPU1_Task.d" 

ASM_DEPS__QUOTED += \
"f2838x_codestartbranch.d" \
"f2838x_usdelay.d" 

C_SRCS__QUOTED += \
"../CPU1_Main.c" \
"../f2838x_cputimers.c" \
"../f2838x_defaultisr.c" \
"../f2838x_epwm.c" \
"../f2838x_globalvariabledefs.c" \
"../f2838x_gpio.c" \
"../f2838x_piectrl.c" \
"../f2838x_pievect.c" \
"../f2838x_sdfm_drivers.c" \
"../f2838x_struct.c" \
"../f2838x_sysctrl.c" 

ASM_SRCS__QUOTED += \
"../f2838x_codestartbranch.asm" \
"../f2838x_usdelay.asm" 


