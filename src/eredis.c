#include "server.h"
#include "eredis.h"

/* Stuff from server.c */
void initServerConfig(void);
void initServer(void);

int eredis_init(void) {
    uint8_t hashseed[16];
    getRandomBytes(hashseed,sizeof(hashseed));
    dictSetHashFunctionSeed(hashseed);

    server.sentinel_mode = 0;

    initServerConfig();
    ACLInit();

    /* Override configuration */
    server.verbosity = LL_WARNING;
    server.io_threads_active = 0;
    server.port = 0;            /* no tcp */
    server.unixsocket = NULL;   /* no unix domain */
    server.protected_mode = 0;  /* no protected mode */

    moduleInitModulesSystem();
    initServer();
    moduleLoadFromQueue();

    return 0;
}

struct eredis_client {
    client *client;
    int buf_consumed;
    listIter reply_iter;
};

eredis_client_t *eredis_create_client(void)
{
    eredis_client_t *c = zmalloc(sizeof(eredis_client_t));
    c->client = createClient(NULL);
    c->client->flags |= CLIENT_MODULE;      /* So we get replies even with fd == -1 */
    return c;
}

void eredis_free_client(eredis_client_t *c)
{
    freeClient(c->client);
    zfree(c);
}


int eredis_prepare_request(eredis_client_t *c, int args_count, const char **args, size_t *arg_lens)
{
    client *rc = c->client;
    resetClient(rc);

    rc->argv = zmalloc(sizeof(robj *) * args_count);
    rc->argc = args_count;
    int i;
    for (i = 0; i < rc->argc; i++) {
        size_t len = arg_lens ? arg_lens[i] : strlen(args[i]);
        rc->argv[i] = createStringObject(args[i], len);
    }
    rc->bufpos = 0;
    listEmpty(rc->reply);
    c->buf_consumed = 0;

    return 0;
}

int eredis_execute(eredis_client_t *c)
{
    client *rc = c->client;

    if (processCommand(rc) != C_OK) return -1;

    return 0;
}

int eredis_prepare_cmd(eredis_client_t *c, const char *cmd)
{
    client *rc = c->client;
    int i;
    int argc;
    sds *argv;

    freeClientArgv(rc);

    // Split the command in arguments
    argv = sdssplitargs(cmd, &argc);

    if (argc) {
        rc->argv = zmalloc(sizeof(robj*)*argc);
    }

    for (rc->argc = 0, i = 0; i < argc; i++) {
        if (sdslen(argv[i])) {
            rc->argv[rc->argc] = createObject(OBJ_STRING, argv[i]);
            rc->argc++;
        } else {
            sdsfree(argv[i]);
        }
    }
    zfree(argv);

    // free old reply
    rc->bufpos = 0;
    listEmpty(rc->reply);
    c->buf_consumed = 0;

    return 0;
}

const char *eredis_read_reply_chunk(eredis_client_t *c, int *chunk_len)
{
    if (!c->buf_consumed) {
        listRewind(c->client->reply, &c->reply_iter);
        c->buf_consumed = 1;
        if (c->client->bufpos) {
            *chunk_len = c->client->bufpos;
            return c->client->buf;
        }
    }

    listNode *curr;
    clientReplyBlock *o;

    while ((curr = listNext(&c->reply_iter)) != NULL) {
        o = listNodeValue(curr);
        if (o->used == 0) continue;
        *chunk_len = o->used;

        return o->buf;
    }

    *chunk_len = 0;
    return NULL;
}
