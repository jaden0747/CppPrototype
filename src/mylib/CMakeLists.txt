file(GLOB_RECURSE SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# The IPC subsystem and the FIFO logger scene that depends on it are POSIX-only
# (mkfifo, O_NONBLOCK, SIGPIPE, EPIPE/EAGAIN, ssize_t). On Windows those lessons
# are taught with completely different APIs (CreateNamedPipe, GetLastError),
# so we drop these sources from the Windows build instead of shimming them.
if(WIN32)
  list(FILTER SOURCES EXCLUDE REGEX "/ipc/[^/]+\\.cpp$")
  list(FILTER SOURCES EXCLUDE REGEX "/customscene\\.cpp$")
endif()

set(PROJECT_INCLUDE_DIRS ${PROJECT_INCLUDE_DIRS} PARENT_SCOPE)

set(PROJECT_SOURCES ${PROJECT_SOURCES} ${SOURCES} PARENT_SCOPE)
