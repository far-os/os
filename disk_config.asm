qi_magic: dd 0xc091fa2b ; qi magic
exec: ; executable block
        dd %!KERN_SIZE + 1
        db 0x1
        
qi_end:
        times 510-($-qi_magic) db 0 ; pad to the 510th byte

