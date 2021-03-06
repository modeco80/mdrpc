#pragma once
#include <stdint.h>

#ifdef __cplusplus
#include <functional>
extern "C" {
#endif

//#define DISCORD_DISABLE_IO_THREAD

typedef struct DiscordRichPresence {
    const char* state;   /* max 128 bytes */
    const char* details; /* max 128 bytes */
    int64_t startTimestamp;
    int64_t endTimestamp;
    const char* largeImageKey;  /* max 32 bytes */
    const char* largeImageText; /* max 128 bytes */
    const char* smallImageKey;  /* max 32 bytes */
    const char* smallImageText; /* max 128 bytes */
    const char* partyId;        /* max 128 bytes */
    int partySize;
    int partyMax;
    const char* matchSecret;    /* max 128 bytes */
    const char* joinSecret;     /* max 128 bytes */
    const char* spectateSecret; /* max 128 bytes */
    int8_t instance;
} DiscordRichPresence;

typedef struct DiscordUser {
    const char* userId;
    const char* username;
    const char* discriminator;
    const char* avatar;
} DiscordUser;

typedef struct DiscordEventHandlers {
#if 0
    void (*ready)(const DiscordUser* request);
    void (*disconnected)(int errorCode, const char* message);
    void (*errored)(int errorCode, const char* message);
    void (*joinGame)(const char* joinSecret);
    void (*spectateGame)(const char* spectateSecret);
    void (*joinRequest)(const DiscordUser* request);
#else
    // hack so I can use bind (and therefore, use class member functions)
    std::function<void(const DiscordUser*)> ready;
    std::function<void(int, const char*)> disconnected;
    std::function<void(int, const char*)> errored;
    std::function<void(const char*)> joinGame;
    std::function<void(const char*)> spectateGame;
    std::function<void(const DiscordUser*)> joinRequest;
#endif
} DiscordEventHandlers;

#define DISCORD_REPLY_NO 0
#define DISCORD_REPLY_YES 1
#define DISCORD_REPLY_IGNORE 2

void Discord_Initialize(const char* applicationId,
                                       DiscordEventHandlers* handlers,
                                       int autoRegister,
                                       const char* optionalSteamId);
void Discord_Shutdown(void);

/* checks for incoming messages, dispatches callbacks */
void Discord_RunCallbacks(void);

/* If you disable the lib starting its own io thread, you'll need to call this from your own */
#ifdef DISCORD_DISABLE_IO_THREAD
 void Discord_UpdateConnection(void);
#endif

void Discord_UpdatePresence(const DiscordRichPresence* presence);
void Discord_ClearPresence(void);

void Discord_Respond(const char* userid, /* DISCORD_REPLY_ */ int reply);

void Discord_UpdateHandlers(DiscordEventHandlers* handlers);

#ifdef __cplusplus
} /* extern "C" */
#endif
