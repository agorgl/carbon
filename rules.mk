RES_FOLDER = res
RES_FILES  = $(call filter-files, $(call rwildcard, $(RES_FOLDER), *))
RES_DFILE  = $(BUILDDIR)/res.dat

ifneq ($(RES_FILES),)
$(RES_DFILE): $(RES_FILES)
	@$(info $(DCYAN_COLOR)[+] Wrapping$(NO_COLOR) $(DYELLOW_COLOR)$(RES_FOLDER)/$(NO_COLOR))
	$(showcmd)tar -C $(RES_FOLDER) -cf $@ $(subst $(RES_FOLDER)/,,$(wildcard $(RES_FOLDER)/*))

$(RES_DFILE)$(OBJEXT): $(RES_DFILE)
ifneq ($(HOST_OS), Darwin)
	$(showcmd)cd $(@D) && ld -r -b binary -o $(@F) $(basename $(@F))
else
	$(showcmd)cd $(@D) && echo "int _res_stub = 0;" > res_stub.c && gcc -o res_stub.o -c res_stub.c
	$(showcmd)ld -r -o $@ -sectcreate __DATA __res_dat $(RES_DFILE) $(@D)/res_stub.o
endif

OBJ_$(D) += $(RES_DFILE)$(OBJEXT)
endif
