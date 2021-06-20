
set(srcs
    "node32.c"
    "sensing.c")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ".")
