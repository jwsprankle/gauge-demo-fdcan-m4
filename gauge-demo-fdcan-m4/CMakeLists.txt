cmake_minimum_required(VERSION 3.15)

project(gauge-demo-fdcan-m4 LANGUAGES C CXX ASM)

find_bsp(ID com.sysprogs.arm.stm32mp1
	VERSION 2020.01
	MCU STM32MP157C_M4
	CONFIGURATION com.sysprogs.bspoptions.arm.floatmode=-mfloat-abi=hard
	FRAMEWORKS com.sysprogs.arm.stm32.hal com.sysprogs.arm.stm32.freertos com.sysprogs.arm.stm32.openamp
	FWCONFIGURATION com.sysprogs.bspoptions.stm32.freertos.portcore=CM4F com.sysprogs.bspoptions.stm32.hal_legacy=
	HWREGISTER_LIST_FILE STM32MP1xxxx/DeviceDefinitions/stm32mp157cxx_cm4.xml
	C_STANDARD 99)

bsp_include_directories(Inc
	${BSP_ROOT}/VendorSamples/MP1/Drivers/BSP/STM32MP15xx_DISCO
 	${BSP_ROOT}/STM32MP1xxxx/STM32MP1xx_HAL_Driver/Inc
	${BSP_ROOT}/STM32MP1xxxx/STM32MP1xx_HAL_Driver/Inc/Legacy
	${BSP_ROOT}/FreeRTOS/Source/portable/GCC/ARM_CM4F
	${BSP_ROOT}/FreeRTOS/Source/include
	${BSP_ROOT}/FreeRTOS/Source/CMSIS_RTOS
	${BSP_ROOT}/FreeRTOS/Source/portable/GCC/ARM_CM4F
	${BSP_ROOT}/STM32MP1xxxx/CMSIS_HAL/Device/ST/STM32MP1xx/Include
	${BSP_ROOT}/STM32MP1xxxx/CMSIS_HAL/Include
	${BSP_ROOT}/VendorSamples/MP1/Utilities/ResourcesManager)

add_bsp_based_executable(NAME gauge-demo-fdcan-m4
	GENERATE_BIN
	GENERATE_MAP
	SOURCES
		${BSP_ROOT}/VendorSamples/MP1/Drivers/BSP/STM32MP15xx_DISCO/stm32mp15xx_disco.c
		${BSP_ROOT}/OpenAMP/virtual_driver/virt_uart.c
		copro_sync.c
		lock_resource.c
		main.c
		mbox_ipcc.c
		obd2_fdcan.c
		obd2_openamp.c
		openamp.c
		openamp_log.c
		rsc_table.c
		stm32mp1xx_hal_msp.c
		stm32mp1xx_hal_timebase_tim.c
		stm32mp1xx_it.c
		system_stm32mp1xx.c
		copro_sync.h
		FreeRTOSConfig.h
		lock_resource.h
		main.h
		mbox_ipcc.h
		obd2_fdcan.h
		obd2_openamp.h
		openamp.h
		openamp_conf.h
		openamp_log.h
		rsc_table.h
		stm32mp1xx_hal_conf.h
		stm32mp1xx_it.h
		stm32mp15xx_disco_conf.h)
