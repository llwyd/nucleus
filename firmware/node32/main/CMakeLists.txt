
set(srcs
    "node32.c"
    "sensing.c"
    "comms.c")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ".")
