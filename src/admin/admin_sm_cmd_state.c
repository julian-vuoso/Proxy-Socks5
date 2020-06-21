#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/sctp.h>
#include <errno.h>

#include "commands.h"
#include "adminmt.h"
#include "admin.h"
#include "logger.h"

void admin_cmd_init(const unsigned state, struct selector_key *key) {
    struct admin_cmd_st * st = &ADMIN_ATTACH(key)->client.cmd;
    st->read_buf = &(ADMIN_ATTACH(key)->read_buffer);
    st->write_buf = &(ADMIN_ATTACH(key)->write_buffer);
    admin_parser_init(&st->parser);
}

void admin_cmd_close(const unsigned state, struct selector_key *key) {
    struct admin_cmd_st * st = &ADMIN_ATTACH(key)->client.cmd;
    admin_parser_close(&st->parser);
}


unsigned admin_cmd_process(struct selector_key *key) {
    struct admin_cmd_st * st_vars = &ADMIN_ATTACH(key)->client.cmd;
    unsigned ret = ADMIN_CMD;

    struct admin_data_word word;
    exec_cmd_and_answ(st_vars->parser.error, st_vars->parser.data, &word);
    if (admin_marshall(st_vars->write_buf, word.value, word.length))
        ret = ADMIN_ERROR;
    
    return ret;
}

unsigned admin_cmd_read(struct selector_key *key) {
    struct admin_cmd_st * st_vars = &ADMIN_ATTACH(key)->client.cmd;
    unsigned ret = ADMIN_CMD;
    bool errored = false;
    size_t nbytes;
    uint8_t * buf_write_ptr = buffer_write_ptr(st_vars->read_buf, &nbytes);
    ssize_t n = sctp_recvmsg(key->fd, buf_write_ptr, nbytes, NULL, NULL, NULL, NULL);

    if (n > 0) {
        buffer_write_adv(st_vars->read_buf, n);
        do {
            const enum admin_state state = admin_consume(st_vars->read_buf, &st_vars->parser, &errored);
            if (admin_is_done(state, 0)) {
                if (selector_add_interest(key->s, key->fd, OP_WRITE) == SELECTOR_SUCCESS){
                    if (admin_cmd_process(key) == ADMIN_ERROR) break;
                    admin_parser_reset(&st_vars->parser);
                } else {
                    ret = ADMIN_ERROR;
                    break;
                }
            }
        } while (buffer_can_read(st_vars->read_buf));
    } else ret = ADMIN_ERROR;

    return ret;
}

unsigned admin_cmd_write(struct selector_key *key) {
    struct admin_cmd_st * st_vars = &ADMIN_ATTACH(key)->client.cmd;
    unsigned ret = ADMIN_CMD;
    size_t nbytes;
    uint8_t * buf_read_ptr = buffer_read_ptr(st_vars->write_buf, &nbytes);
    ssize_t n = sctp_sendmsg(key->fd, buf_read_ptr, nbytes, NULL, 0, 0, 0, 0, 0, 0);

    if (n > 0) {
        buffer_read_adv(st_vars->write_buf, n);
        if (!buffer_can_read(st_vars->write_buf)) { // Finish sending message
            if (selector_remove_interest(key->s, key->fd, OP_WRITE) == SELECTOR_SUCCESS)
                ret = ADMIN_CMD;
            else ret = ADMIN_ERROR;
        }
    } else {
        logger_log(DEBUG, "admin error en command write\n\nError. errno message: %s\n\n", strerror(errno));
        ret = ADMIN_ERROR;
    }
    return ret;
}