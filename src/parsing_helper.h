#ifndef PARSING_HELPER_H
#define PARSING_HELPER_H

#if defined(_WIN32)
#ifndef TARGET_WINDOWS
#define TARGET_WINDOWS
#endif
#else
#ifndef TARGET_LINUX
#define TARGET_LINUX
#endif
#endif

#ifndef TARGET_IA32
#define TARGET_IA32
#endif

#ifndef HOST_IA32
#define HOST_IA32
#endif

#ifndef USING_XED
#define USING_XED
#endif

#endif // PARSING_HELPER_H

