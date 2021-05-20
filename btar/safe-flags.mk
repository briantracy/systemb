
#
# safe-flags.mk -- helpful compiler flags for safer C code
#

############ Sources ############
# fortify source: https://access.redhat.com/blogs/766093/posts/1976213
# code gen options: https://gcc.gnu.org/onlinedocs/gcc/Code-Gen-Options.html
# more f options: https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html
# https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
# https://developers.redhat.com/blog/2018/03/21/compiler-and-linker-flags-gcc/
# https://gcc.gnu.org/onlinedocs/gcc/Code-Gen-Options.html
# https://gcc.gnu.org/onlinedocs/gcc/index.html#Top


# What language are we using?
LANGFLAGS := -std=c11 -Wpedantic

# This should enable a good base of warnings
BASEWARN := -Wall -Wextra

# Warnings related to arithmetic
ARITHWARN := -Wdouble-promotion -Wconversion -Wint-conversion

# String formatting vulnerability prevention
FORMATWARN := -Wformat=2 -Wformat-truncation -Wformat-overflow=2 \
				-Wformat-nonliteral -Wformat-security -Wformat-signedness

# Macro related warnings
MACROWARN := -Wundef -Wunused-macros

# Prevent unaligned casting and const erasure
CASTWARN := -Wcast-qual -Wcast-align=strict

# Semantic issues
SEMWARN := -Wnull-dereference

# Source level issues
# Note that some of these may produce false positives. You might have to
# reduce the verbosity level if that happens.
SOURCEWARN := \
	-Wshadow -Wduplicated-branches -Wduplicated-cond \
	-Wmissing-include-dirs -Wshift-overflow=2 \
	-Wswitch-default -Wswitch-enum \
	-Wunused-parameter -Wuninitialized -Wstrict-overflow=4 \
	-Wstringop-overflow=4 -Walloc-zero -Walloca \
	-Wfloat-equal -Wwrite-strings -Wjump-misses-init -Wlogical-op \
	-Wpacked -Wredundant-decls -Wnested-externs -Winline \
	-Wunsuffixed-float-constants -Wvla


WARNINGS := $(BASEWARN) $(ARITHWARN) $(FORMATWARN) \
			$(MACROWARN) $(CASTWARN) $(SEMWARN) $(SOURCEWARN)

# -Og is recommended for standard debug builds
OPTIMIZATIONFLAGS   := -O2 -Wunsafe-loop-optimizations -Wno-aggressive-loop-optimizations

# DOES NOT PLAY NICE WITH SANITIZERS
FOPTIONS   := -fstack-protector-strong

# Any additional #defines
DEFINES    := -D_FORTIFY_SOURCE=3

#
SANITIZERS := -fsanitize=address -fsanitize=undefined

# Static analyzer only available for gcc-10 and newer
ANALYZERS  := -fanalyzer

# Embed debug information
DEBUGFLAGS := -ggdb3


# Actually export our changes
CFLAGS += $(LANGFLAGS) $(WARNINGS) $(OPTIMIZATIONFLAGS)  \
			$(DEFINES) $(SANITIZERS) $(DEBUGFLAGS)




