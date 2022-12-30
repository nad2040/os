
char buf[BUFLEN];
size_t nbytes = 0;
char *curpos = buf;

while(1) {
    poll();
    //POLLIN
    int n = read(fd, buf+nbytes, BUFLEN-nbytes);
    processBuf(curpos, nbytes);

    if (nbytes == 0) break;
    memmove(buf, curpos, nbytes);
}

struct Header {
    size_t len;
    size_t seqNo;
};
inline void processBuf(char*& curpos, size_t nbytes) {
    while(hasUnitOfWork(curpos, nbytes)) {
        processUnitOfWork(curpos, nbytes);
    }
}

inline bool hasUnitOfWor(char *curpos, size_t nbytes) {
    return nbytes > sizeof(Header) && (Header*)curpos->len + sizeof(Header) <= nbytes;
}

inline void processUnitOfWork(char*& curpos, size_t& nbytes) {
    size_t len = (Header*)curpos->len;
    char *msg = curpos + sizeof(Header);
    processMsg(msg, len);
    size_t unitLen = len + sizeof(Header);
    curpos += unitLen;
    nbytes -= unitLen;
}
