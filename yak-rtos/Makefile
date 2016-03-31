#-------------------------------------------------------------------------
# Makefile
# 	- Description: Application Makefile
#-------------------------------------------------------------------------
FILE = application

$(FILE).bin: $(FILE)_final.s
		nasm $(FILE)_final.s -o $(FILE).bin -l $(FILE).lst                                          # Assemble

$(FILE)_final.s: clib.s yak_asm.s isrs.s yak_c.s yak_help.s handlers.s $(FILE).s
		cat clib.s yak_asm.s isrs.s yak_c.s yak_help.s handlers.s $(FILE).s > $(FILE)_final.s       # Concatenate

yak_c.s: yak_c.c
		cpp yak_c.c yak_c.i                                                                         # Preprocess
		c86 -g yak_c.i yak_c.s                                                                      # Compile

yak_help.s: yak_help.c
		cpp yak_help.c yak_help.i                                                                   # Preprocess
		c86 -g yak_help.i yak_help.s                                                                # Compile

handlers.s: handlers.c
		cpp handlers.c handlers.i                                                                   # Preprocess
		c86 -g handlers.i handlers.s                                                                # Compile

$(FILE).s: $(FILE).c
		cpp $(FILE).c $(FILE).i                                                                     # Preprocess
		c86 -g $(FILE).i $(FILE).s                                                                  # Compile
		
clean:	
		rm $(FILE).bin $(FILE).lst $(FILE)_final.s yak_c.s yak_c.i yak_help.s yak_help.i handlers.s handlers.i $(FILE).s $(FILE).i
