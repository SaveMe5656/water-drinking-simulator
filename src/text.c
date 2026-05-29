#include <string.h>
#include <fontlibc.h>
#include <fileioc.h>

char *wds_getFont(char *fontName)
{
	char *appVar;
	uint8_t *appVar_ptr = NULL;
	char *appVar_fontName;

	// loop through font pack AppVars
	while ((appVar = ti_Detect(&appVar_ptr, "FONTPACK")) != NULL)
	{
		// attempt to fetch font name
		appVar_fontName = fontlib_GetFontPackName(appVar);
		// skip if font being checked is unnamed
		if (!appVar_fontName)
			continue;
		// set font and return if name matches requested
		if (!strcmp(appVar_fontName, fontName))
			return appVar;
	}
	return NULL;
}
