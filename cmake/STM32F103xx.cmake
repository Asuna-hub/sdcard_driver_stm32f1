set(CPU_PARAMETERS ${CPU_PARAMETERS}
    -mthumb
    -mcpu=cortex-m3
    -mfloat-abi=soft
)

set(compiler_define ${compiler_define}
    "STM32F103xB"
)