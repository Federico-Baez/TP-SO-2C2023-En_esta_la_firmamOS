################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := Memoria
BUILD_ARTIFACT_EXTENSION :=
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: main-build

# Main-build Target
main-build: Memoria

dependents:
	-cd /home/utnso/tp-2023-2c-En_esta_la_firmamOS/Shared/Debug && $(MAKE) all

# Tool invocations
Memoria: $(OBJS) $(USER_OBJS) /home/utnso/tp-2023-2c-En_esta_la_firmamOS/Shared/Debug/libShared.so makefile $(OPTIONAL_TOOL_DEPS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/tp-2023-2c-En_esta_la_firmamOS/Shared/Debug" -o "Memoria" $(OBJS) $(USER_OBJS) $(LIBS) -lcommons -lpthread -lShared
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) Memoria
	-@echo ' '

.PHONY: all clean dependents main-build
/home/utnso/tp-2023-2c-En_esta_la_firmamOS/Shared/Debug/libShared.so:

-include ../makefile.targets
