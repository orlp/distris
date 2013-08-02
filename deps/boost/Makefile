NEUTRAL_SOURCES = \
	src/atomic/lockpool.cpp \
	src/date_time/gregorian/date_generators.cpp \
	src/date_time/gregorian/gregorian_types.cpp \
	src/date_time/gregorian/greg_month.cpp \
	src/date_time/gregorian/greg_weekday.cpp \
	src/date_time/posix_time/posix_time_types.cpp \
	src/exception/clone_current_exception_non_intrusive.cpp \
	src/regex/cpp_regex_traits.cpp \
	src/regex/cregex.cpp \
	src/regex/c_regex_traits.cpp \
	src/regex/fileiter.cpp \
	src/regex/icu.cpp \
	src/regex/instances.cpp \
	src/regex/posix_api.cpp \
	src/regex/regex.cpp \
	src/regex/regex_debug.cpp \
	src/regex/regex_raw_buffer.cpp \
	src/regex/regex_traits_defaults.cpp \
	src/regex/static_mutex.cpp \
	src/regex/usinstances.cpp \
	src/regex/w32_regex_traits.cpp \
	src/regex/wc_regex_traits.cpp \
	src/regex/wide_posix_api.cpp \
	src/regex/winstances.cpp \
	src/smart_ptr/sp_collector.cpp \
	src/smart_ptr/sp_debug_hooks.cpp \
	src/system/error_code.cpp \
	src/thread/future.cpp \
	src/thread/tss_null.cpp

WIN_SOURCES = \
	src/thread/win32/thread.cpp \
	src/thread/win32/tss_dll.cpp \
	src/thread/win32/tss_pe.cpp

LINUX_SOURCES = \
	src/thread/pthread/once.cpp \
	src/thread/pthread/once_atomic.cpp \
	src/thread/phtread/thread.cpp

ifeq ($(OS),Windows_NT)
	SOURCES = $(NEUTRAL_SOURCES) $(WIN_SOURCES)
else
	SOURCES = $(NEUTRAL_SOURCES) $(LINUX_SOURCES)
endif

OBJS = $(patsubst %.cpp, %.o, $(SOURCES))

libboost.a: $(OBJS)
	ar rcs libboost.a $(OBJS)

src/%.o: src/%.cpp
	g++ -O3 -Iinclude -c $< -o $@