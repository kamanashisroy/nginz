#ifndef NGINZ_SCANNER
#define NGINZ_SCANNER

C_CAPSULE_START

/**
 * NOTE the next needs to be cleaned up by aroop_txt_destroy(next) call
 */
int scanner_next_token (aroop_txt_t* src, aroop_txt_t* next);
int scanner_trim (aroop_txt_t* src, aroop_txt_t* trimmed);

C_CAPSULE_END

#endif // NGINZ_SCANNER
