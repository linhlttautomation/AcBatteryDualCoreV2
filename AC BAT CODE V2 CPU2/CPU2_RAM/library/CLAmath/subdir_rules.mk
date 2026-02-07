################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
library/CLAmath/%.obj: ../library/CLAmath/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs1250/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla2 --float_support=fpu64 --idiv_support=idiv0 --tmu_support=tmu0 --vcu_support=vcrc -Ooff --include_path="C:/Users/levuo/ver2_ACBat/AC BAT CODE V2 CPU2/library/Driver/include" --include_path="C:/Users/levuo/ver2_ACBat/AC BAT CODE V2 CPU2/library/Driver/source" --include_path="C:/Users/levuo/ver2_ACBat/AC BAT CODE V2 CPU2/include" --include_path="C:/Users/levuo/ver2_ACBat/AC BAT CODE V2 CPU2/library/Driver" --include_path="C:/Users/levuo/ver2_ACBat/AC BAT CODE V2 CPU2/library/Driver/include" --include_path="C:/Users/levuo/ver2_ACBat/AC BAT CODE V2 CPU2/library/App_lib/Transform/include" --include_path="C:/Users/levuo/ver2_ACBat/AC BAT CODE V2 CPU2/library/App_lib/PWM/include" --include_path="C:/Users/levuo/ver2_ACBat/AC BAT CODE V2 CPU2/library/App_lib/Phaselockedloop/include" --include_path="C:/Users/levuo/ver2_ACBat/AC BAT CODE V2 CPU2/library/App_lib/Other/include" --include_path="C:/Users/levuo/ver2_ACBat/AC BAT CODE V2 CPU2/library/App_lib/Observer/include" --include_path="C:/Users/levuo/ver2_ACBat/AC BAT CODE V2 CPU2/library/App_lib/MPPT/include" --include_path="C:/Users/levuo/ver2_ACBat/AC BAT CODE V2 CPU2/library/App_lib/Motor/include" --include_path="C:/Users/levuo/ver2_ACBat/AC BAT CODE V2 CPU2/library/App_lib/Controller/include" --include_path="C:/Users/levuo/ver2_ACBat/AC BAT CODE V2 CPU2" --include_path="C:/ti/c2000/C2000Ware_6_00_00_00/device_support/f2838x/common/include" --include_path="C:/ti/c2000/C2000Ware_6_00_00_00/device_support/f2838x/headers/include" --include_path="C:/ti/ccs1250/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include" --define=DEBUG --define=CPU2 --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="library/CLAmath/$(basename $(<F)).d_raw" --obj_directory="library/CLAmath" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


