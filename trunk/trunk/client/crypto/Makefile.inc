
!if "$(TC_AES_EXT)" == "x86"
TC_OBJ_FORMAT = win32
!else
TC_OBJ_FORMAT = win64
!endif

"$(OBJ_PATH)\$(O)\Aes_$(TC_AES_EXT).obj": Aes_$(TC_AES_EXT).asm
	yasm.exe -Xvc -f $(TC_OBJ_FORMAT) -D DLL_EXPORT -a $(TC_ARCH) -o "$@" Aes_$(TC_AES_EXT).asm
