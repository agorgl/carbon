PRJTYPE = StaticLib
VERSION = 3.3.2
SRC = \
	src/context.c \
	src/init.c    \
	src/input.c   \
	src/monitor.c \
	src/vulkan.c  \
	src/window.c
ifeq ($(TARGET_OS), Windows)
	SRC += \
		src/win32_init.c     \
		src/win32_joystick.c \
		src/win32_monitor.c  \
		src/win32_time.c     \
		src/win32_thread.c   \
		src/win32_window.c   \
		src/wgl_context.c    \
		src/egl_context.c    \
		src/osmesa_context.c \
		src/null_joystick.c
else ifeq ($(TARGET_OS), Linux)
	SRC += \
		src/x11_init.c       \
		src/x11_monitor.c    \
		src/x11_window.c     \
		src/xkb_unicode.c    \
		src/posix_time.c     \
		src/posix_thread.c   \
		src/glx_context.c    \
		src/egl_context.c    \
		src/osmesa_context.c \
		src/linux_joystick.c
else ifeq ($(TARGET_OS), Darwin)
	SRC += \
		src/cocoa_init.m     \
		src/cocoa_joystick.m \
		src/cocoa_monitor.m  \
		src/cocoa_window.m   \
		src/cocoa_time.c     \
		src/posix_thread.c   \
		src/nsgl_context.m   \
		src/egl_context.c    \
		src/osmesa_context.c \
		src/null_joystick.c
else
	SRC += \
		src/null_init.c      \
		src/null_monitor.c   \
		src/null_window.c    \
		src/null_joystick.c  \
		src/posix_time.c     \
		src/posix_thread.c   \
		src/osmesa_context.c \
		src/null_joystick.c
endif

ifeq ($(TARGET_OS), Windows)
	DEFINES += _GLFW_WIN32
else ifeq ($(TARGET_OS), Linux)
	DEFINES += _GLFW_X11
else ifeq ($(TARGET_OS), Darwin)
	DEFINES += _GLFW_COCOA
endif

ifeq ($(TARGET_OS), Windows)
	LIBS += glu32 opengl32 gdi32 winmm ole32 user32 shell32
else ifeq ($(TARGET_OS), Linux)
	LIBS += GL GLU X11 Xrandr Xinerama Xcursor pthread dl
endif

ifeq ($(TARGET_OS), Darwin)
	MLDFLAGS += -framework Cocoa -framework IOKit -framework CoreFoundation -framework CoreVideo
endif
