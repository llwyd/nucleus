
set(srcs
    "node32.c"
    "sensing.c"
    "comms.c"
    "dataqueue.c"
    "mqtt.c"
    "weather.c"
    "bme280.c")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ".")
