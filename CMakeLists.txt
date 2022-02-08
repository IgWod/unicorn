# CMake setup for Unicorn 2.
# By Huitao Chen & Nguyen Anh Quynh, 2019-2020

cmake_minimum_required(VERSION 3.1)

# Detect if unicorn is compiled as the top-level project
set(PROJECT_IS_TOP_LEVEL OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(PROJECT_IS_TOP_LEVEL ON)
    
    # Enable folder support
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

project(unicorn C)

set(UNICORN_VERSION_MAJOR 2)
set(UNICORN_VERSION_MINOR 0)
set(UNICORN_VERSION_PATCH 0)

option(BUILD_SHARED_LIBS "Build shared instead of static library" OFF)
option(UNICORN_FUZZ "Enable fuzzing" OFF)
option(UNICORN_BUILD_TESTS "Build unicorn tests" OFF)
option(UNICORN_INSTALL "Enable unicorn installation" ${PROJECT_IS_TOP_LEVEL})
option(UNICORN_TRACER "Trace unicorn execution" OFF)

include_directories(
	${CMAKE_BINARY_DIR}
)

include_directories(
    glib_compat
    qemu
    qemu/include
    include
    qemu/tcg
)

set(UNICORN_TARGET_ARCH "aarch64")

set(EXTRA_CFLAGS "--extra-cflags=")
set(EXTRA_CFLAGS "${EXTRA_CFLAGS}-DUNICORN_HAS_ARM64 ")

set(EXTRA_CFLAGS "${EXTRA_CFLAGS}-fPIC")

set(TARGET_LIST "--target-list=")
set(TARGET_LIST "${TARGET_LIST}aarch64-softmmu, ")

# GEN config-host.mak & target directories
execute_process(COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/qemu/configure
	--cc=${CMAKE_C_COMPILER}
	${EXTRA_CFLAGS}
	${TARGET_LIST}
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

execute_process(COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/qemu/scripts/create_config
	INPUT_FILE ${CMAKE_BINARY_DIR}/config-host.mak
	OUTPUT_FILE ${CMAKE_BINARY_DIR}/config-host.h
)

execute_process(COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/qemu/scripts/create_config
	INPUT_FILE ${CMAKE_BINARY_DIR}/aarch64-softmmu/config-target.mak
	OUTPUT_FILE ${CMAKE_BINARY_DIR}/aarch64-softmmu/config-target.h
)

add_compile_options(
	${UNICORN_CFLAGS}
	-I${CMAKE_CURRENT_SOURCE_DIR}/qemu/tcg/${UNICORN_TARGET_ARCH}
	-D_GNU_SOURCE
	-D_FILE_OFFSET_BITS=64
	-D_LARGEFILE_SOURCE
	-Wall
	-fPIC
)

set(UNICORN_ARCH_COMMON
    qemu/exec.c
    qemu/exec-vary.c

    qemu/softmmu/cpus.c
    qemu/softmmu/ioport.c
    qemu/softmmu/memory.c
    qemu/softmmu/memory_mapping.c

    qemu/fpu/softfloat.c

    qemu/tcg/optimize.c
    qemu/tcg/tcg.c
    qemu/tcg/tcg-op.c
    qemu/tcg/tcg-op-gvec.c
    qemu/tcg/tcg-op-vec.c

    qemu/accel/tcg/cpu-exec.c
    qemu/accel/tcg/cpu-exec-common.c
    qemu/accel/tcg/cputlb.c
    qemu/accel/tcg/tcg-all.c
    qemu/accel/tcg/tcg-runtime.c
    qemu/accel/tcg/tcg-runtime-gvec.c
    qemu/accel/tcg/translate-all.c
    qemu/accel/tcg/translator.c
)

set(UNICORN_AARCH64_SRCS
    ${UNICORN_ARCH_COMMON}

    qemu/target/arm/cpu64.c
    qemu/target/arm/cpu.c
    qemu/target/arm/crypto_helper.c
    qemu/target/arm/debug_helper.c
    qemu/target/arm/helper-a64.c
    qemu/target/arm/helper.c
    qemu/target/arm/iwmmxt_helper.c
    qemu/target/arm/m_helper.c
    qemu/target/arm/neon_helper.c
    qemu/target/arm/op_helper.c
    qemu/target/arm/pauth_helper.c
    qemu/target/arm/psci.c
    qemu/target/arm/sve_helper.c
    qemu/target/arm/tlb_helper.c
    qemu/target/arm/translate-a64.c
    qemu/target/arm/translate.c
    qemu/target/arm/translate-sve.c
    qemu/target/arm/vec_helper.c
    qemu/target/arm/vfp_helper.c
    qemu/target/arm/unicorn_aarch64.c
)

set(UNICORN_SRCS
    lift.c
    uc.c
    qemu/softmmu/vl.c
    qemu/hw/core/cpu.c
)

set(UNICORN_COMMON_SRCS

    list.c

    glib_compat/glib_compat.c
    glib_compat/gtestutils.c
    glib_compat/garray.c
    glib_compat/gtree.c
    glib_compat/grand.c
    glib_compat/glist.c
    glib_compat/gmem.c
    glib_compat/gpattern.c
    glib_compat/gslice.c

    qemu/util/bitmap.c
    qemu/util/bitops.c
    qemu/util/crc32c.c
    qemu/util/cutils.c
    qemu/util/getauxval.c
    qemu/util/guest-random.c
    qemu/util/host-utils.c
    qemu/util/osdep.c
    qemu/util/qdist.c
    qemu/util/qemu-timer.c
    qemu/util/qemu-timer-common.c
    qemu/util/range.c
    qemu/util/qht.c
    qemu/util/pagesize.c
    qemu/util/cacheinfo.c

    qemu/crypto/aes.c
)

set(UNICORN_COMMON_SRCS
	${UNICORN_COMMON_SRCS}
	qemu/util/oslib-posix.c
	qemu/util/qemu-thread-posix.c
)

add_library(unicorn
    ${UNICORN_SRCS}
	${UNICORN_COMMON_SRCS}
	${UNICORN_AARCH64_SRCS}
)

target_compile_options(unicorn PRIVATE
    ${UNICORN_COMPILE_OPTIONS}
	-DNEED_CPU_H
	-include aarch64.h
	-I${CMAKE_BINARY_DIR}/aarch64-softmmu
	-I${CMAKE_CURRENT_SOURCE_DIR}/qemu/target/arm
)

set_target_properties(unicorn PROPERTIES
	VERSION ${UNICORN_VERSION_MAJOR}
	SOVERSION ${UNICORN_VERSION_MAJOR}
)

target_include_directories(unicorn PUBLIC
    include
)

add_library(unishare SHARED
		${UNICORN_SRCS}
		${UNICORN_COMMON_SRCS}
		${UNICORN_AARCH64_SRCS}
)

target_compile_options(unishare PRIVATE
		${UNICORN_COMPILE_OPTIONS}
		-DNEED_CPU_H
		-include aarch64.h
		-I${CMAKE_BINARY_DIR}/aarch64-softmmu
		-I${CMAKE_CURRENT_SOURCE_DIR}/qemu/target/arm
		)

set_target_properties(unishare PROPERTIES
		VERSION ${UNICORN_VERSION_MAJOR}
		SOVERSION ${UNICORN_VERSION_MAJOR}
		)

target_include_directories(unishare PUBLIC
		include
		)

if(UNICORN_INSTALL AND NOT MSVC)
    include("GNUInstallDirs")
    file(GLOB UNICORN_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/unicorn/*.h)
    install(TARGETS unicorn
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
    install(FILES ${UNICORN_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/unicorn)
    file(WRITE ${CMAKE_BINARY_DIR}/unicorn.pc "Name: unicorn\n\
Description: Unicorn emulator engine\n\
Version: ${UNICORN_VERSION_MAJOR}.${UNICORN_VERSION_MINOR}.${UNICORN_VERSION_PATCH}\n\
libdir=${CMAKE_INSTALL_FULL_LIBDIR}\n\
includedir=${CMAKE_INSTALL_FULL_INCLUDEDIR}\n\
Libs: -L\$\{libdir\} -lunicorn\n\
Cflags: -I\$\{includedir\}\n"
    )
    install(FILES ${CMAKE_BINARY_DIR}/unicorn.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()
