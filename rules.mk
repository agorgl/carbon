RES_FOLDER = res
RES_FILES  = $(call filter-files, $(call rwildcard, $(RES_FOLDER), *))
RES_DFILE  = res.dat

$(BUILDDIR)/$(RES_DFILE)$(OBJEXT): $(RES_FILES)
	@$(info $(DCYAN_COLOR)[+] Wrapping$(NO_COLOR) $(DYELLOW_COLOR)$(RES_FOLDER)/$(NO_COLOR))
	$(showcmd)tar -C $(RES_FOLDER) -cf $(basename $@) $(subst $(RES_FOLDER)/,,$(wildcard $(RES_FOLDER)/*))
	$(showcmd)cd $(@D) && ld -r -b binary -o $(@F) $(basename $(@F))

OBJ_$(D) += $(BUILDDIR)/$(RES_DFILE)$(OBJEXT)
