/**
 * @file        K_WppTracing.hpp
 * @brief       WppTracing header
 * @author      NAVAL-Group (Berenger BRAULT, Nicolas JALLET)
 * @version     1.0
 * @date        02/06/2022
 * @copyright   ©Naval Group SA.
 *              This document in its content and form is the property of Naval Group SA and/or third parties.
 *              This project is released under the LGPLv3 license.
*/


//
// Software tracing Definitions
//

// Note: default print function => DoTraceMessage(TraceFlagName, Message)

#define WPP_CONTROL_GUIDS									\
	WPP_DEFINE_CONTROL_GUID(								\
		Ctrl, (9DA60C9A, 65F2, 4991, BB11, 87FBB8B190CD),	\
		WPP_DEFINE_BIT(INIT)	/* bit 0 = 0x00000001 */	\
		WPP_DEFINE_BIT(CONF)	/* bit 1 = 0x00000002 */	\
		WPP_DEFINE_BIT(REG)		/* bit 2 = 0x00000004 */	\
		WPP_DEFINE_BIT(COMM)	/* bit 3 = 0x00000008 */	\
		WPP_DEFINE_BIT(FLT)		/* bit 4 = 0x00000010 */	\
		WPP_DEFINE_BIT(WLIST)	/* bit 5 = 0x00000020 */	\
		WPP_DEFINE_BIT(CACH)	/* bit 6 = 0x00000040 */	\
		WPP_DEFINE_BIT(OBCB)	/* bit 7 = 0x00000080 */	\
		)

#define WPP_LEVEL_FLAGS_LOGGER(lvl, flags) \
	WPP_LEVEL_LOGGER(flags)

#define WPP_LEVEL_FLAGS_ENABLED(lvl, flags) \
	(WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level >= lvl)

//
// This comment block is scanned by the trace preprocessor to define the
// TraceEvents function.
//
// begin_wpp config
// FUNC TraceEvents(LEVEL, FLAGS, MSG, ...);
// end_wpp
