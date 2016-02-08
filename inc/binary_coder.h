#ifndef NGINEZ_BINARY_CODER_H
#define NGINEZ_BINARY_CODER_H

C_CAPSULE_START

int binary_coder_reset(aroop_txt_t*buffer);
/*
 * It concats string data to the buffer, Note that the message cannot be bigger than 255 bytes.
 * @param buffer The buffer where the string is concatenated
 * @param x The string to add
 * @return 0 when successful, -1 otherwise
 */
int binary_concat_string(aroop_txt_t*buffer, aroop_txt_t*x);

C_CAPSULE_END

#endif // NGINEZ_BINARY_CODER_H
