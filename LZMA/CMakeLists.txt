SET(LIB_NAME LZMA)

SET(C_FILES
	Alloc.c
	LzFind.c
	LzFindMt.c
	LzmaDec.c
	LzmaEnc.c
	LzmaLib.c
)
IF(WIN32)
	SET(C_FILES ${C_FILES}
		Util/LzmaLib/LzmaLibExports.c
	)
ENDIF()

SET(DEF_FILES
	Util/LzmaLib/LzmaLib.def
)
SET(LZMA_HEADER_FILES
	Alloc.h
	LzFind.h
	LzFindMt.h
	LzHash.h
	LzmaDec.h
	LzmaEnc.h
	LzmaLib.h
	Threads.h
	Types.h
)
SOURCE_GROUP("Source Files" FILES ${DEF_FILES} ${C_FILES})
SOURCE_GROUP("Header Files" FILES ${LZMA_HEADER_FILES})

ADD_DEFINITIONS(-DNO_ASM)
ADD_DEFINITIONS(-DLZMALIB_EXPORTS)
ADD_DEFINITIONS(-D_7ZIP_ST)

ADD_LIBRARY(${LIB_NAME} "SHARED"
	${C_FILES} ${DEF_FILES} ${LZMA_HEADER_FILES}
)