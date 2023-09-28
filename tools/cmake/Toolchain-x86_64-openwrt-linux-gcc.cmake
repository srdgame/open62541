# the name of the target operating system
set(CMAKE_SYSTEM_NAME Linux)

# which compilers to use for C and C++
set(CMAKE_C_COMPILER x86_64-openwrt-linux-gcc)
set(CMAKE_CXX_COMPILER x86_64-openwrt-linux-g++)

set(MBEDTLS_FOLDER_INCLUDE "$ENV{STAGING_DIR}/usr/include")
set(MBEDTLS_FOLDER_LIBRARY "$ENV{STAGING_DIR}/usr/lib")
set(OPENSSL_ROOT_DIR "$ENV{STAGING_DIR}/usr")
