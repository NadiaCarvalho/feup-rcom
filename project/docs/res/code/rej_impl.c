/* Create a BCC2 for the I frame
check if the received one is correct*/
char calculated_bcc2 = 0;
int i;
for (i = 0; i < *packet_len; i++)
  calculated_bcc2 ^= packet[i];

if (calculated_bcc2 ==
    expected_bcc2) { // valid BCC2 - may still be a duplicate

  reply = create_US_frame(&reply_len, RR);

  /* Only need to check sequence number if packet is a data packet.
  * If it is, and the sequence number is invalid, discard the packet
  * by setting its length to 0 */
  if (!has_valid_sequence_number(frame[2], s)) {
    *packet_len = 0;
    printf("Found duplicate frame. Discarding...\n");
  } else {
    //Only flip sequence number if the whole frame is valid
    //And not a duplicate.
    s = !s;
  }

  read_succesful = 1;
} else { // BCC2 does not match -> check sequence number
  if (has_valid_sequence_number(frame[2], s)) { // new frame, request retry
    reply = create_US_frame(&reply_len, REJ);
    printf("Found new incorrect frame. Rejecting...\n");
  } else {
    reply = create_US_frame(&reply_len,
                            RR); // duplicate frame, send RR and discard
    read_succesful = 1;
    *packet_len = 0;
    printf("Found duplicate frame. Discarding...\n");
  }
}

if (write_to_tty(fd, reply, reply_len) != 0) {
  printf("Error write_to_tty() in function llread().\n");
  return -1;
}
