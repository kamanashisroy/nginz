#ifndef NGINZ_BINARY_CODER_H
#define NGINZ_BINARY_CODER_H

C_CAPSULE_START
/*
 * It clears the old data in buffer
 * @param buffer The buffer, NOTE it needs the buffer to be at least 256 bytes long, ie, aroop_txt_embeded_stackbuffer(&create_msg, 255);
 */
int binary_coder_reset(aroop_txt_t*buffer);
int binary_coder_reset_for_pid(aroop_txt_t*buffer, int destpid);
/*
 * It concats string data to the buffer, Note that the message cannot be bigger than 255 bytes.
 * @param buffer The buffer where the string is concatenated
 * @param x The string to add
 * @return 0 when successful, -1 otherwise
 */
int binary_pack_string(aroop_txt_t*buffer, aroop_txt_t*x);
/*
 * It gets the next string data out of the buffer
 * @param buffer The buffer where the string is contained
 * @param x The output string
 * @return 0 when successful, -1 otherwise
 */
int binary_unpack_string(aroop_txt_t*buffer, int skip, aroop_txt_t*x);
int binary_unpack_int(aroop_txt_t*buffer, int skip, int*intval);

int binary_coder_fixup(aroop_txt_t*buffer);
int binary_coder_debug_dump(aroop_txt_t*buffer);

C_CAPSULE_END

#endif // NGINZ_BINARY_CODER_H
