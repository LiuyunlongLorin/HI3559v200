# Describe the include path, using the full path

# Enum the C files needed to be compiled, using the relative path


# pm domian switch,depended on chip model
# hi355av100 pm domain config
# y: on
# n: off

CONFIG_PM_DOMAIN_CORE_ON    :=y
CONFIG_PM_DOMAIN_CPU_ON     :=n
CONFIG_PM_DOMAIN_MEDIA_ON   :=y

# pm regulator type select
# y: dc-dc/pwm
# n: pmu/i2c

CONFIG_PM_REGULATOR_DCDC   :=y
