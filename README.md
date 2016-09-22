# tftp_server

Trivial File Transfer Protocol (TFTP) server

The specification of TFTP is in RFC1350 available at http://tools.ietf.org/html/rfc1350.

Any request to read a filename that starts with a question mark will result in the directory listing being returned. 
A filename of “?” will return the listing for the root directory of the TFTP server.

A file transfer request that has a star symbol in the filename will specify that an offset is to be used, the characters
after the star will specify the offset in decimal. 
For example a read request of “filename*1024” will return filename starting at an offset of 1024 bytes into the file.
