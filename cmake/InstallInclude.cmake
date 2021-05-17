# utilities
set(DESTINATION_INCLUDE_DIR "${CMAKE_INSTALL_INCLUDEDIR}/bcos-framework")
install(
    DIRECTORY "libutilities"
    DESTINATION "${DESTINATION_INCLUDE_DIR}"
    FILES_MATCHING PATTERN "*.h"
)

# interfaces
install(
    DIRECTORY "interfaces"
    DESTINATION "${DESTINATION_INCLUDE_DIR}"
    FILES_MATCHING PATTERN "*.h"
)

# codec
install(
    DIRECTORY "libcodec"
    DESTINATION "${DESTINATION_INCLUDE_DIR}"
    FILES_MATCHING PATTERN "*.h"
)

# protocol
install(
    DIRECTORY "libprotocol"
    DESTINATION "${DESTINATION_INCLUDE_DIR}"
    FILES_MATCHING PATTERN "*.h"
)

install(
    DIRECTORY ${PROTO_GENERATE_DIR}
    DESTINATION "${DESTINATION_INCLUDE_DIR}/libprotocol"
    FILES_MATCHING PATTERN "*.h"
)

# table
install(
    DIRECTORY "libtable"
    DESTINATION "${DESTINATION_INCLUDE_DIR}"
    FILES_MATCHING PATTERN "*.h"
)

#sync
install(
    DIRECTORY "libsync"
    DESTINATION "${DESTINATION_INCLUDE_DIR}"
    FILES_MATCHING PATTERN "*.h"
)
#sealer
install(
    DIRECTORY "libsealer"
    DESTINATION "${DESTINATION_INCLUDE_DIR}"
    FILES_MATCHING PATTERN "*.h"
)
#testutils
install(
    DIRECTORY "testutils"
    DESTINATION "${DESTINATION_INCLUDE_DIR}"
    FILES_MATCHING PATTERN "*.h"
)