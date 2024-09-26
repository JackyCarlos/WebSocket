/***************************************************************************//**

  @file         utils.h

  @author       Robert Eikmanns

  @date         Thursday, 7 March 2024

  @brief        Declarations for helper functions

*******************************************************************************/

char *split(char *str, const char *delim);
int get_listener_socket(char *host_address, char *port);
int recv_bytes(int fd, uint8_t *mem, uint32_t fetch_bytes);