
DIAG=diag.log
BTEST=../../../../btest/btest

all: cleanup btest-verbose

# Showing all tests.
btest-verbose:
	@$(BTEST) -j -f $(DIAG)

brief: cleanup btest-brief

# Brief output showing only failed tests.
btest-brief:
	@$(BTEST) -j -b -f $(DIAG)

# Rerun only the failed tests.
rerun:
	@$(BTEST) -r -j -f $(DIAG)

cleanup:
	@rm -f $(DIAG)
	@rm -f .btest.failed.dat
	@rm -rf .tmp

.PHONY: all btest-verbose btest-brief brief rerun cleanup
