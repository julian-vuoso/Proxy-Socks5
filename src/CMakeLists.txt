cmake_minimum_required(VERSION 2.8)
include_directories(../include)
include_directories(../include/admin)
include_directories(../include/doh)
include_directories(../include/parsers)
include_directories(../include/sm)
include_directories(../include/utils)
include_directories(../include/client)

add_executable(socks5d main.c
                       utils/users.c
                       parsers/hello.c
                       parsers/negotiation.c
                       utils/args.c
                       utils/base64.c
                       parsers/admin.c
                       parsers/ettercap.c
                       utils/parser.c
                       utils/parser_utils.c
                       parsers/request.c
                       utils/buffer.c
                       utils/selector.c
                       utils/stm.c
                       socks5.c
                       utils/logger.c
                       utils/netutils.c
                       utils/commands.c
                       sm/sm_hello_state.c
                       sm/sm_negot_state.c
                       sm/sm_request_state.c
                       sm/sm_copy_state.c
                       sm/sm_connect_state.c
                       doh/dohParser.c
                       doh/dohclient.c
                       sm/sm_actions.c
                       sm/socks5mt.c
                       admin/admin_sm_cmd_state.c
                       admin/admin_sm_negot_state.c
                       admin/admin_socks5.c)
target_link_libraries(socks5d pthread sctp)

add_executable(client client/client.c client/clientUtils.c)
target_link_libraries(client sctp)